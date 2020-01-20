#include <stdio.h>
#include "Defines.h"
#include "SendingThread.h"
#include "MessageQueue.h"
#include "Loger.h"

#pragma warning(disable : 4996)

DWORD WINAPI SendingThread_Run(LPVOID pParams)
{
	PSendingThreadParams pThreadParams = (PSendingThreadParams)pParams;
	while (TRUE)
	{
		char *message = NULL;

		// Dequeue
		int dequeueSuccess = MessageQueue_Dequeue(pThreadParams->messageQueue, &message);// will wait here till receive a message
		if (SUCCESS_CODE != dequeueSuccess)
		{
			exit(FAILURE_CODE);
		}
		CHAR tempStringToLoger[MESSAGE_MAX_LENGTH] = { 0 };
		int sprintfResult = sprintf(tempStringToLoger, "Sent to server:%s\n", message);
		if ((-1) == sprintfResult)
		{
			printf("failed on make string to loger\n");
			exit(FAILURE_CODE);
		}
		Loger_WriteToLogAndPrint(tempStringToLoger, TRUE);
		
		// Send loop
		int current_offset = 0;
		int total_bytes_num = strlen(message) + 1;
		while (current_offset < total_bytes_num)
		{
			int sendResult = send(pThreadParams->serverConnection, message + current_offset, total_bytes_num - current_offset, 0);
			if (SOCKET_ERROR == sendResult)
			{
				printf(" %s sent successfuly\n", message + current_offset);
				if (WSAGetLastError() == WSAECONNRESET) 
				{
					CHAR strToLoger[MESSAGE_MAX_LENGTH] = { 0 };
					int sprintfRes = sprintf(strToLoger, "server disconnected. Exiting.");
					if ((-1) == sprintfRes)
					{
						printf("failed on make string to loger\n");
						exit(FAILURE_CODE);
					}
					Loger_WriteToLogAndPrint(strToLoger, FALSE);
				}
				free(message);
				exit(FAILURE_CODE);
			}
			current_offset += sendResult;
		}

		free(message);
	}
}