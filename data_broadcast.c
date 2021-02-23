#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <net/if.h>

#include "get_network_cards_name.h"
int port = 6000;
int server_port = 6001;
void *main(void *params)
{
	int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(socket_fd < 0){
		printf("socket create failed\n");
		return NULL;
	}
	int opt = 1;
	int ret =setsockopt(socket_fd,SOL_SOCKET,SO_BROADCAST,(void *)&opt,sizeof(opt));
	if(ret < 0){
		printf("setsockopt failed\n");
		return NULL;
	}
	struct sockaddr_in addrto;
	bzero(&addrto,sizeof(struct sockaddr_in));
	addrto.sin_family=AF_INET;
	addrto.sin_addr.s_addr=htonl(INADDR_BROADCAST);//套接字地址为广播地址
	addrto.sin_port=htons(port);//套接字广播端口号
	int nlen=sizeof(addrto);
	char msg[64]= {0};

	string_array *card_devices = get_network_cards_name();
	char buf[16] = {0};
	get_ipaddress(card_devices->str[1], buf);
	printf("%s\n", buf);
	sprintf(msg, "{\"ip\":\"%s\",\"port\":%d}", buf, server_port);

	while(1){
		int ret=sendto(socket_fd,msg,strlen(msg),0,(struct sockaddr*)&addrto,nlen);//向广播地址发布消息
		if(ret<0){
			printf("broadcast failed\n");
		}
		printf("%s\n", msg);
		sleep(5);
	}
}
