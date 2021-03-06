#include "socker_server.h"

SocketServer::SocketServer()
{
	socker_server_init();
}

int SocketServer::socket_server_init()
{
	struct sockaddr_in client; /* client address information          */
    struct sockaddr_in server; /* server address information          */
	if ((socket_accept = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket()");
        exit(2);
    }
	/*
     * Bind the socket to the server address.
     */
    server.sin_family = AF_INET;
    server.sin_port   = htons(port);
    server.sin_addr.s_addr = INADDR_ANY;	
	if (bind(socket_server, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        tcperror("Bind()");
        exit(3);
    }
}
