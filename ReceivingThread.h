#ifndef RECEIVING_THREAD_H
#define RECEIVING_THREAD_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <windows.h>
#include <wS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#include "MessageQueue.h"
#include "Defines.h"

/*
Description - ReceivngThreadParams struct holds parameters for receiving thread

*/
typedef struct _ReceivngThreadParams
{
	SOCKET serverConnection;
	HANDLE clientTurnEvent;
	CHAR clientUsername[USER_NAME_MAX_LENGTH];
} ReceivngThreadParams, *PReceivingThreadParams;

/*
Description - ReceivingThread_Run function runs the receiving thread
Parameters  -
input       : pParams - parameters of the thread
returns     : DWORD handle for thread
*/
DWORD WINAPI ReceivingThread_Run(LPVOID pParams);

#endif