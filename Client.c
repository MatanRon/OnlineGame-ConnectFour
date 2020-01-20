#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include "Defines.h"
#include "ProcessUtils.h"
#include "Client.h"
#include "UIThread.h"
#include "SendingThread.h"
#include "ReceivingThread.h"
#include "Loger.h"

#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable : 4996)

typedef enum
{
	UI_THREAD_INDEX = 0,
	SENDER_THREAD_INDEX,
	RECEIVER_THREAD_INDEX,

	CLIENT_NUM_OF_THREADS
} ClientThreadsIndexes;

void Client_Cleanup(PQueue messageQueue, PHANDLE ClientThreadsArray, SOCKET serverConnection)
{
	if (SOCKET_ERROR != serverConnection)
	{
		(VOID)closesocket(serverConnection);
	}
	MessageQueue_Cleanup(messageQueue);
	for (int threadIndex = 0; threadIndex < CLIENT_NUM_OF_THREADS; threadIndex++)
	{
		// Best effort 
		if (NULL != ClientThreadsArray[threadIndex])
		{
			(VOID)TerminateThread(ClientThreadsArray[threadIndex], TERMINATED_CODE);
			(VOID)CloseHandle(ClientThreadsArray[threadIndex]);
		}
	}
}

int Client_GetReceivingThreadParams(PReceivingThreadParams *ppReceivingThreadParams, SOCKET serverConnection, HANDLE clientTurnEvent, CHAR *clientUsername)
{
	//creat an instance of PReceivingThreadParams 
	PReceivingThreadParams pReceivingThreadParams = NULL;

	pReceivingThreadParams = (PReceivingThreadParams)calloc(1, sizeof(*pReceivingThreadParams));
	if (NULL == pReceivingThreadParams)
	{
		Loger_WriteToLogAndPrint("Custom message: memory alocation for Receiving thread params struct faild\n", FALSE);
		exit(FAILURE_CODE);
	}
	pReceivingThreadParams->serverConnection = serverConnection;
	pReceivingThreadParams->clientTurnEvent = clientTurnEvent;
	strcpy(pReceivingThreadParams->clientUsername, clientUsername);
	*ppReceivingThreadParams = pReceivingThreadParams;
	return SUCCESS_CODE;
}


int Client_GetUIThreadParams(PUIThreadParams *ppUIThreadParams, FILE *inputFile, PQueue senderQueue, HANDLE clientTurnEvent)
{
	//creat an instance of PUIThreadParams 
	PUIThreadParams pUIThreadParams = NULL;

	pUIThreadParams = (PUIThreadParams)calloc(1, sizeof(*pUIThreadParams));
	if (NULL == pUIThreadParams)
	{
		Loger_WriteToLogAndPrint("Custom message: memory alocation for UI thread params struct faild\n", FALSE);
		exit(FAILURE_CODE);
	}
	pUIThreadParams->inputFile = inputFile;
	pUIThreadParams->senderQueue = senderQueue;
	pUIThreadParams->clientTurnEvent = clientTurnEvent;

	*ppUIThreadParams = pUIThreadParams;
	return SUCCESS_CODE;
}


int Client_GetSendingThreadParams(PSendingThreadParams *ppSendingThreadParams, SOCKET serverConnection, PQueue senderQueue)
{
	//creat an instance of PSendingThreadParams 
	PSendingThreadParams pSendingThreadParams = NULL;

	pSendingThreadParams = (PSendingThreadParams)calloc(1, sizeof(*pSendingThreadParams));
	if (NULL == pSendingThreadParams)
	{
		Loger_WriteToLogAndPrint("Custom message: memory alocation for sending thread params struct faild \n", FALSE);
		exit(FAILURE_CODE);
	}
	pSendingThreadParams->messageQueue = senderQueue;
	pSendingThreadParams->serverConnection = serverConnection;

	*ppSendingThreadParams = pSendingThreadParams;
	return SUCCESS_CODE;
}

int Client_RunClientThreads(PHANDLE *pClientThreadsArray, FILE *inputFile, SOCKET serverConnection, PQueue senderQueue, CHAR *userName)
{
	PUIThreadParams pUIThreadParams = NULL;
	PSendingThreadParams pSendingThreadParams = NULL;
	PReceivingThreadParams pReceivingThreadParams = NULL;

	HANDLE clientTurnEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (NULL == clientTurnEvent)
	{
		Loger_WriteToLogAndPrint("Custom message: failed in creating event", FALSE);
		exit(FAILURE_CODE);
	}

	//allocate UI thread params struct and put values in its attributes
	int getUIParamsResult = Client_GetUIThreadParams(&pUIThreadParams, inputFile, senderQueue, clientTurnEvent);
	if (SUCCESS_CODE != getUIParamsResult)
	{
		Loger_WriteToLogAndPrint("Custom message: failed on get UI thread params\n", FALSE);
		exit(FAILURE_CODE);
	}
	//allocate sending thread params struct and put values in its attributes
	int getsenderParamsResult = Client_GetSendingThreadParams(&pSendingThreadParams, serverConnection, senderQueue);
	if (SUCCESS_CODE != getsenderParamsResult)
	{
		Loger_WriteToLogAndPrint("Custom message: failed on get UI thread params\n", FALSE);
		exit(FAILURE_CODE);
	}

	//allocate receiving thread params struct and put values in its attributes
	int getReceiverParamsResult = Client_GetReceivingThreadParams(&pReceivingThreadParams, serverConnection, clientTurnEvent, userName);
	if (SUCCESS_CODE != getReceiverParamsResult)
	{
		Loger_WriteToLogAndPrint("Custom message: failed on get receiving thread params\n", FALSE);
		exit(FAILURE_CODE);
	}

	PHANDLE clientThreadsArray = (PHANDLE)calloc(CLIENT_NUM_OF_THREADS, sizeof(clientThreadsArray[0]));//that gonna be a size of HANDLE
	if (NULL == clientThreadsArray)
	{
		Loger_WriteToLogAndPrint("Custom message: clientThreadsArray mem alloc failure\n", FALSE);
		exit(FAILURE_CODE);
	}
	clientThreadsArray[UI_THREAD_INDEX] = BeginThreadExSimple(UIThread_Run, pUIThreadParams);
	if (NULL == clientThreadsArray[UI_THREAD_INDEX])
	{
		Loger_WriteToLogAndPrint("Custom message: failed on begining UserInterfaceThread thread\n", FALSE);
		exit(FAILURE_CODE);
	}
	clientThreadsArray[SENDER_THREAD_INDEX] = BeginThreadExSimple(SendingThread_Run, pSendingThreadParams);
	if (NULL == clientThreadsArray[SENDER_THREAD_INDEX])
	{
		Loger_WriteToLogAndPrint("Custom message: failed on begining sending thread\n", FALSE);
		exit(FAILURE_CODE);
	}

	clientThreadsArray[RECEIVER_THREAD_INDEX] = BeginThreadExSimple(ReceivingThread_Run, pReceivingThreadParams);
	if (NULL == clientThreadsArray[RECEIVER_THREAD_INDEX])
	{
		Loger_WriteToLogAndPrint("Custom message: failed on begining receiver thread\n", FALSE);
		exit(FAILURE_CODE);
	}
	*pClientThreadsArray = clientThreadsArray;
	return SUCCESS_CODE;
}

int Client_SendUserNameRequest(BOOL isHuman, FILE *inputFile, PQueue senderQueue, PCHAR outUserName)
{
	CHAR message[MESSAGE_MAX_LENGTH] = { 0 };
	char userName[USER_NAME_MAX_LENGTH];

	if (isHuman)
	{
		printf("Please enter a user name\n");
	}

	//if not human mode the input file is stdin so the next line shuld happen both in user mod and file mode.
	char *fgetsResult = fgets(userName, USER_NAME_MAX_LENGTH, inputFile);
	if (NULL == fgetsResult)
	{
		printf("failed on getting a user name");
	}
	SIZE_T userNameLength = strlen(userName);
	if ('\n' == userName[userNameLength - 1])
	{
		userName[userNameLength - 1] = '\0';
	}
	strcpy(outUserName, userName);

	strcpy(message, "NEW_USER_REQUEST:");
	strcat(message, userName);

	CHAR *fullMessage = _strdup(message);
	if (NULL == fullMessage)
	{
		Loger_WriteToLogAndPrint("Custom message: failed on strdup \n", FALSE);
		exit(FAILURE_CODE);
	}
	int enqueResult = MessageQueue_Enqueue(senderQueue, fullMessage);
	if (SUCCESS_CODE != enqueResult)
	{
		free(fullMessage);
		Loger_WriteToLogAndPrint("Custom message: failed on enqueuemwssage\n", FALSE);
		exit(FAILURE_CODE);
	}
	return SUCCESS_CODE;
}

int Client_Run(unsigned short serverPortAddr, FILE *inputFile, BOOL isHuman)
{
	PQueue senderQueue = NULL;
	SOCKET serverConnection = INVALID_SOCKET;
	PHANDLE ClientThreadsArray = NULL;
	struct sockaddr_in server;
	//char *message;

	serverConnection = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == serverConnection)
	{
		Loger_WriteToLogAndPrint("Custom message: Could not create socket\n", FALSE);
		exit(FAILURE_CODE);
	}

	printf("Socket created.\n");
	INT inet_pton_res = inet_pton(AF_INET, "127.0.0.1", &(server.sin_addr));
	if (1 != inet_pton_res)
	{
		Loger_WriteToLogAndPrint("Custom message: Could not translate ip to in_addr struct\n", FALSE);
		exit(FAILURE_CODE);
	}
	server.sin_family = AF_INET;
	server.sin_port = htons(serverPortAddr);

	//Connect to remote server
	if (connect(serverConnection, (struct sockaddr *)&server, sizeof(server)) != 0)
	{
		CHAR stringToLoger[MESSAGE_MAX_LENGTH] = { 0 };
		int sprintfResult = sprintf(stringToLoger, "Failed connecting to server on 127.0.0.1:%d.Exiting", serverPortAddr);
		if ((-1) == sprintfResult)
		{
			printf("failed on make string to loger\n");
			exit(FAILURE_CODE);
		}
		Loger_WriteToLogAndPrint(stringToLoger, FALSE);
		exit(FAILURE_CODE);
	}

	CHAR stringToLoger[MESSAGE_MAX_LENGTH] = { 0 };
	int sprintfResult = sprintf(stringToLoger,"%s %d\n", "Connected to server on 127.0.0.1:", serverPortAddr);
	if ((-1) == sprintfResult)
	{
		printf("failed on make string to loger");
		exit(FAILURE_CODE);
	}
	Loger_WriteToLogAndPrint(stringToLoger, FALSE);

	int queueCreateResult = MessageQueue_Create(&senderQueue);
	if (SUCCESS_CODE != queueCreateResult)
	{
		Loger_WriteToLogAndPrint("Custom message: failed on creat senderQueue\n", FALSE);
		exit(FAILURE_CODE);
	}

	CHAR userName[USER_NAME_MAX_LENGTH] = { 0 };
	Client_SendUserNameRequest(isHuman, inputFile, senderQueue, userName);

	int runThreadsResults = Client_RunClientThreads(&ClientThreadsArray, inputFile, serverConnection, senderQueue, userName);
	if (SUCCESS_CODE != runThreadsResults)
	{
		printf("failed on run client threads");
		exit(FAILURE_CODE);
	}

	// Wait for the threads to finish - wait for multiple, If one finished - all finished(think if that's true).
	DWORD waitResult = WaitForMultipleObjects(CLIENT_NUM_OF_THREADS, ClientThreadsArray, FALSE, INFINITE);
	DWORD exitCode = (DWORD)FAILURE_CODE;
	BOOL success = FALSE;
	switch (waitResult)
	{
	case WAIT_OBJECT_0 + UI_THREAD_INDEX:
		success = GetExitCodeThread(ClientThreadsArray[UI_THREAD_INDEX], &exitCode);
		if (!success)
		{
			Loger_WriteToLogAndPrint("Custom message: Failed getting exit code of ui thread\n", FALSE);
			exit(FAILURE_CODE);
		}
		if (SUCCESS_CODE != exitCode)
		{
			Loger_WriteToLogAndPrint("Custom message: Exit code of ui thread was not success code\n", FALSE);
			exit(FAILURE_CODE);
		}
		break;
	case WAIT_OBJECT_0 + SENDER_THREAD_INDEX:
		success = GetExitCodeThread(ClientThreadsArray[SENDER_THREAD_INDEX], &exitCode);
		if (!success)
		{
			Loger_WriteToLogAndPrint("Custom message: Failed getting exit code of sender thread\n", FALSE);
			exit(FAILURE_CODE);
		}
		if (SUCCESS_CODE != exitCode)
		{
			Loger_WriteToLogAndPrint("Custom message: Exit code of sender thread was not success code\n", FALSE);
			exit(FAILURE_CODE);
		}
		break;
	case WAIT_OBJECT_0 + RECEIVER_THREAD_INDEX:
		success = GetExitCodeThread(ClientThreadsArray[RECEIVER_THREAD_INDEX], &exitCode);
		if (!success)
		{
			Loger_WriteToLogAndPrint("Custom message: Failed getting exit code of receiver thread\n", FALSE);
			exit(FAILURE_CODE);
		}
		if (SUCCESS_CODE != exitCode)
		{
			Loger_WriteToLogAndPrint("Custom message: Exit code of receiver thread was not success code\n", FALSE);
			exit(FAILURE_CODE);
		}
		break;
	case WAIT_TIMEOUT:
		Loger_WriteToLogAndPrint("Custom message: Waiting on client thread array timeout\n", FALSE);
		exit(FAILURE_CODE);
	default:
		Loger_WriteToLogAndPrint("Custom message: waiting on client threads array failed\n", FALSE);
		exit(FAILURE_CODE);
	}

	Client_Cleanup(senderQueue, ClientThreadsArray, serverConnection);
	return SUCCESS_CODE;
}