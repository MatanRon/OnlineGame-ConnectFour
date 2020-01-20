#ifndef SENDING_THREAD_H
#define SENDING_THREAD_H
#include <windows.h>
#include "MessageQueue.h"

/*
Description - struct for sending thread parameters
*/
typedef struct _SendingThreadParams
{
	SOCKET serverConnection;
	PQueue messageQueue;
} SendingThreadParams, *PSendingThreadParams;




/*
# Description - SendingThread_Run function run the sending thread
# Parameters - pParams: parameters for the thread
# Returns - If succeeded, returns a handle to the newly created thread
*/
DWORD WINAPI SendingThread_Run(LPVOID pParams);


#endif
