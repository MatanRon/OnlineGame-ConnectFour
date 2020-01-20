#include "Socket.h"

void Socket_CreateSockadderServer(SOCKADDR_IN *service, unsigned long *address, int serverPort)
{
	(*service).sin_family = AF_INET;
	(*service).sin_addr.s_addr = *address;
	(*service).sin_port = htons(serverPort);
	return;
}