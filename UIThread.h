#ifndef UI_THREAD_H
#define UI_THREAD_H

#include <windows.h>
#include "Defines.h"
#include "MessageQueue.h"

typedef struct _UIThreadParams
{
	PQueue senderQueue;
	FILE *inputFile;
	HANDLE clientTurnEvent;
} UIThreadParams, *PUIThreadParams;

/*
# Description - UIThread_Run function runs handles all messages from/to server.
# Parameters - pParams: parameters for the thread
# Returns - If succeeded, returns a handle to the newly created thread
*/
DWORD WINAPI UIThread_Run(LPVOID lpParams);

#endif

