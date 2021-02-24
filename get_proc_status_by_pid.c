#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
 
char *proc_status_key[] = {
	"VmPeak",
	"VmSize",
	"VmLck",
	"VmPin",
	"VmHWM",
	"VmRSS",
	"RssAnon",
	"RssFile",
	"RssShmem",
	"VmData",
	"VmStk",
	"VmExe",
	"VmLib",
	"VmPTE",
	"VmSwap",
	"Cpus_allowed"
};

int find_pid_by_name( char* ProcName, int* foundpid)
{
        DIR             *dir;
        struct dirent   *d;
        int             pid, i;
        char            *s;
        int pnlen;
 
        i = 0;
        foundpid[0] = 0;
        pnlen = strlen(ProcName);
 
        /* Open the /proc directory. */
        dir = opendir("/proc");
        if (!dir)
        {
                printf("cannot open /proc");
                return -1;
        }
 
        /* Walk through the directory. */
        while ((d = readdir(dir)) != NULL) {
 
                char exe [PATH_MAX+1];
                char path[PATH_MAX+1];
                int len;
                int namelen;
 
                /* See if this is a process */
                if ((pid = atoi(d->d_name)) == 0)       continue;
 
                snprintf(exe, sizeof(exe), "/proc/%s/exe", d->d_name);
				//readlink exe: system interface to get ProcName abs path
                if ((len = readlink(exe, path, PATH_MAX)) < 0)
                        continue;
                path[len] = '\0';
 
                /* Find ProcName */
                s = strrchr(path, '/');
                if(s == NULL) continue;
                s++;
 
                /* we don't need small name len */
                namelen = strlen(s);
                if(namelen < pnlen)     continue;
 
                if(!strncmp(ProcName, s, pnlen)) {
                        /* to avoid subname like search proc tao but proc taolinke matched */
                        if(s[pnlen] == ' ' || s[pnlen] == '\0') {
                                foundpid[i] = pid;
                                i++;
                        }
                }
        }
 
        foundpid[i] = 0;
        closedir(dir);
 
        return i;
}

char *space_jump(const char *str)
{
	char *p = str;
	for(;*p == ' '; p++);
	return p;
}
 
void proc_status_content_handle(void *params, const char *status_str)
{
	int max = sizeof(proc_status_key)/sizeof(char *);
	for(int i = 0; i < max; i++){
		int len = strlen(proc_status_key[i]);
		if(strncmp(status_str, proc_status_key[i],len) == 0){
			printf("%s", status_str);
			printf("%s", space_jump(&status_str[len+2]));
#if 0
			int l = strlen(status_str);
			for(int j = 0; j < l; j++){
				printf("%c %2x\t", status_str[j], status_str[j]);
			}
			printf("\n");
#endif
		}
	}
}

char *get_proc_status_by_pid(int pid, void(*cbk)(void *, const char *), void *opaque)
{
	if(pid <= 0){
		return NULL;
	}
	char path[64] = {0};
	sprintf(path, "/proc/%d/status", pid);
	FILE *fp = fopen(path, "r");
	if(fp == NULL){
		printf("%s open failed\n", path);
		return NULL;
	}
	char buf[1024] = {0};
	int ret = 0;
	char *gets = NULL;
	while(1){
		//ret = fread(buf, 1, sizeof(buf), fp);
		gets = fgets(buf, sizeof(buf), fp);
		if(gets <= 0){
			break;
		}
		cbk(opaque, (const char *)buf);
		memset(buf, 0, sizeof(buf));
	}
	fclose(fp);
	return NULL;
}

int proc_status_thread(int argc, char **argv)
{
    int ret, pid_t[128];
	ret = find_pid_by_name(argv[1], pid_t);
	for(int i = 0; i < ret; i++){
		get_proc_status_by_pid(pid_t[i], proc_status_content_handle, NULL);
	}
	return 0;
}
