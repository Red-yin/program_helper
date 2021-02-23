#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include "get_network_cards_name.h"

void destory_string_array(string_array *a)
{
	if(a == NULL){
		return;
	}
	for(int i = 0; i < a->num; i++){
		free(a->str[i]);
		a->str[i] = NULL;
	}
}

string_array *get_network_cards_name()
{
	int i=0;
	int sockfd;
	struct ifconf ifc;
	struct ifreq *ifr;
	unsigned char buf[512];
	string_array *ret = (string_array *)calloc(1, sizeof(string_array));
	if(ret == NULL){
		printf("calloc failed\n");
		return NULL;
	}

	//初始化ifconf
	ifc.ifc_len = 512;
	ifc.ifc_buf = buf;

	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0))<0)
	{
		perror("socket");
		exit(1);
	}  
	ioctl(sockfd, SIOCGIFCONF, &ifc);    //获取所有接口信息

	//接下来获取逐个网卡的名称
	ifr = (struct ifreq*)buf;  
	ret->num = (ifc.ifc_len/sizeof(struct ifreq));
	ret->str = (char **)calloc(ret->num, sizeof(char *));
	for(i=0; i < ret->num; i++)
	{
		ret->str[i] = (char *)calloc(1, strlen(ifr->ifr_name) + 1);
		if(ret->str[i] == NULL){
			printf("calloc failed\n");
			goto fail;
		}
		strcpy(ret->str[i], ifr->ifr_name);
		ifr++;
	}

	return ret;
fail:
	if(ret){
		destory_string_array(ret);
	}
	return NULL;
}

int get_ipaddress(char *dev, char *buf)
{
	struct ifreq ifr;
	long int ip;
	int fd;
	struct in_addr tmp_addr;
	strcpy(ifr.ifr_name, dev);
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (ioctl(fd, SIOCGIFADDR, &ifr)) {
		perror("ioctl error");
		return -1;
	}
	close(fd);
	memcpy(&ip,ifr.ifr_addr.sa_data + 2,4);
	tmp_addr.s_addr=ip;
	char *p = inet_ntoa(tmp_addr);
	strncpy(buf, p, 15);
	return 0;
}

#if 0
int getIpAddress(char *iface_name, char *ip_addr, int len)
{
	int sockfd = -1;
	struct ifreq ifr;
	struct sockaddr_in *addr = NULL;

	memset(&ifr, 0, sizeof(struct ifreq));
	strcpy(ifr.ifr_name, iface_name);
	addr = (struct sockaddr_in *)&ifr.ifr_addr;
	addr->sin_family = AF_INET;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("create socket error!\n");
		return -1;
	}

	if (ioctl(sockfd, SIOCGIFADDR, &ifr) == 0) {
		char *p = inet_ntoa(addr->sin_addr);
		strncpy(ip_addr, p, len);
		close(sockfd);
		return 0;
	}

	close(sockfd);

	return -1;
}
#endif
