#include "server_task.h"
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include "vispeech_debug.h"

#define EPOLL_LISTEN_MAX 256
#define EPOLL_LISTEN_TIMEOUT -1
ServerTask::ServerTask()
{
	taskBase = avl_tree_init(NULL);
	epollfdInit(EPOLL_LISTEN_MAX);
	start();
}

ServerTask::~ServerTask()
{
	avl_tree_travel(taskBase, taskContentDestoryOnAvlTree, NULL);
	avl_tree_deinit(&taskBase);
}

void ServerTask::taskContentDestoryOnAvlTree(struct avl_node *node, void *aux)
{
	struct taskContent *task = _get_entry(node, struct taskContent, hook);
	free(task);
}
int ServerTask::epollfdInit(int max)
{
    /* create epoll fd */
    epollfd = epoll_create(max); 
    if (epollfd < 0) {
        VISPEECH_ERR("epoll_create error, Error:[%d:%s]", errno, strerror(errno));
        return -1;
    }
    return epollfd;
}

int ServerTask::epollDeletefd(int fd)
{
    int ret = epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, NULL);
	if(ret < 0){
        VISPEECH_ERR("EPOLL_CTL_DEL error, Error:[%d:%s]", errno, strerror(errno));
	}
	return ret;
}
int ServerTask::epollAddfd(int fd)
{
    int ret;
    struct epoll_event event;

    memset(&event, 0, sizeof(event));
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;

    ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    if(ret < 0) {
        VISPEECH_ERR("epoll_ctl Add fd:%d error, Error:[%d:%s]", fd, errno, strerror(errno));
        return -1;
    }

    return 0;
}

int ServerTask::addTimerTask(int ms, taskHandler *handle, void *param, int repeatFlag)
{
	if(handle == NULL || ms <= 0){
		return -1;
	}

	VISPEECH_DEBUG("add task timer: [%d]\n", ms);
    int ret;
    struct itimerspec new_value;
    new_value.it_value.tv_sec = ms/1000;
    new_value.it_value.tv_nsec = (ms%1000)*1000*1000;
	if(repeatFlag == 1){
		new_value.it_interval.tv_sec = new_value.it_value.tv_sec;
		new_value.it_interval.tv_nsec = new_value.it_value.tv_nsec;
	}else{
		new_value.it_interval.tv_sec = 0;
		new_value.it_interval.tv_nsec = 0;
	}

	int fd = timerfd_create(CLOCK_MONOTONIC, 0);
    ret = timerfd_settime(fd, 0, &new_value, NULL);
    if (ret < 0) {
        VISPEECH_ERR("timerfd_settime error, Error:[%d:%s], fd: %d\n", errno, strerror(errno), fd);
        close(fd);
        return -1;
    }
	if(epollAddfd(fd) != 0){
		return -1;
	}
	
	taskContent *task = (taskContent *)malloc(sizeof(taskContent));
	if(task == NULL){
		VISPEECH_ERR("task malloc failed\n");
		return -1;
	}
	task->fd = fd;
	task->handle = handle;
	task->param = param;
	memset(&task->hook, 0, sizeof(struct avl_node));
	avl_insert(taskBase, &task->hook, compare);
	return fd;
}
int ServerTask::addTask(int fd, taskHandler *handle, void *param)
{
	if(handle == NULL || fd <= 0){
		return -1;
	}

	if(epollAddfd(fd) != 0){
		return -1;
	}
	
	taskContent *task = (taskContent *)malloc(sizeof(taskContent));
	if(task == NULL){
		VISPEECH_ERR("task malloc failed\n");
		return -1;
	}
	task->fd = fd;
	task->handle = handle;
	task->param = param;
	memset(&task->hook, 0, sizeof(struct avl_node));
	avl_insert(taskBase, &task->hook, compare);
	return fd;
}

int ServerTask::compare(struct avl_node *node, struct avl_node *user_data, void *aux)
{
	taskContent *task = _get_entry(user_data, taskContent, hook);
	taskContent *treeTask = _get_entry(node, taskContent, hook);
	if(task->fd == treeTask->fd)
		return 0;
	else if(task->fd > treeTask->fd)
		return 1;
	else
		return -1;
}

taskContent *ServerTask::findTask(int fd)
{
	taskContent cur;
	memset(&cur, 0, sizeof(taskContent));
	cur.fd = fd;
	struct avl_node *result = avl_search(taskBase, &cur.hook, compare);
	if(result == NULL){
		VISPEECH_ERR("%d is not exist\n", fd);
		return NULL;
	}
	return _get_entry(result, taskContent, hook);
}

void ServerTask::printTask(struct avl_node *node, void *aux)
{
	taskContent *task = _get_entry(node, taskContent, hook);
	VISPEECH_DEBUG("task fd: %d\n", task->fd);
}

int ServerTask::scanTask()
{
	avl_tree_travel(taskBase, printTask, NULL);
	return 0;
}


int ServerTask::deleteTask(int fd)
{
	if(fd < 0){
		VISPEECH_ERR("%d is not a correct fd\n", fd);
		return -1;
	}
	int ret;
    ret = epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, NULL);
	if(ret < 0){
        VISPEECH_ERR("EPOLL_CTL_DEL error, Error:[%d:%s]", errno, strerror(errno));
	}
	timerfd_settime(fd, 0, NULL, NULL);
	close(fd);

	taskContent cur;
	memset(&cur, 0, sizeof(cur));
	cur.fd = fd;
	avl_node *nodeT = avl_search(taskBase, &cur.hook, compare);
	if(nodeT == NULL){
		VISPEECH_ERR("%d is not exist\n", fd);
		return ret;
	}
	avl_remove(taskBase, nodeT);
	taskContent *treeTask = _get_entry(nodeT, taskContent, hook);
	free(treeTask);

	return ret;
}

void ServerTask::run()
{
    int i = 0, j = 0;
    int fd_cnt = 0;
    int sfd;
    struct epoll_event events[EPOLL_LISTEN_MAX];    

    memset(events, 0, sizeof(events));
    while(1)
    {   
        /* wait epoll event */
        fd_cnt = epoll_wait(epollfd, events, EPOLL_LISTEN_MAX, EPOLL_LISTEN_TIMEOUT); 
        for(i = 0; i < fd_cnt; i++) 
        {   
            sfd = events[i].data.fd;
            if(events[i].events & EPOLLIN) 
			{
				taskContent *task = findTask(sfd);
				task->handle(task, task->param);
#if 1
				uint64_t exp = 0;
				//if sfd is not read, sfd will be blocked
				read(sfd, &exp, sizeof(uint64_t));
				VISPEECH_DEBUG("read %d: %u\n", sfd, exp);
#endif
			}
        } 
    }   
}
