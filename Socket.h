#ifndef SOCKET_H
#define SOCKET_H

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN

#endif // !WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <wS2tcpip.h>
#include <windows.h>
#include <stdio.h>
#include <string.h>
#pragma comment(lib, "ws2_32.lib")

/*
Description - Socket_CreateSockadderServer function handles and inits all parameters for socket
Parameters  -
input       : SOCKADDR_IN *service, unsigned long *address, int serverport
returns     : none(void function)
*/
void Socket_CreateSockadderServer(SOCKADDR_IN *service, unsigned long *address, int serverport);
#endif // !SOCKET_H