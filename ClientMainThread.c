#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include "Common.h"
#include "Client.h"
#include "UIThread.h"
#include "SendingThread.h"
#include "ProcessUtils.h"
#include "ClientMainThread.h"
typedef enum
{
	UI_THREAD_INDEX = 0,
	SENDER_THREAD_INDEX,
	RECEIVER_THREAD_INDEX,

	CLIENT_NUM_OF_THREADS
} ClientThreadsIndexes;

//ASKME: who call beginThreadExSimple for the CLientmainThread?? who did for mainThread in the last oriject? maybe its dust the thread the program runs with from the begining?

int Client_GetUIThreadParams(PUIThreadParams *ppUIThreadParams,FILE *inputFile, PQueue senderQueue)
{
	//creat an instance of PUIThreadParams 
	PUIThreadParams pUIThreadParams = NULL;

	pUIThreadParams = (PUIThreadParams)calloc(1, sizeof(*pUIThreadParams));
	if (NULL == pUIThreadParams)
	{
		printf("memory alocation for UI thread params struct faild");
		return FAILURE_CODE;
	}
	pUIThreadParams->inputFile = inputFile;
	pUIThreadParams->senderQueue = senderQueue; //TODO ensure passing to this func created senderQueue


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
		printf("memory alocation for sending thread params struct faild");
		return FAILURE_CODE;
	}
	pSendingThreadParams->messageQueue = senderQueue;
	pSendingThreadParams->serverConnection = serverConnection;

	//ASKME: case of ownership transfer ?
	*ppSendingThreadParams = pSendingThreadParams;
	return SUCCESS_CODE;
}

int Client_RunClientThreads(PHANDLE *pClientThreadsArray, FILE *inputFile, SOCKET serverConnection)
{
	PUIThreadParams pUIThreadParams = NULL;
	PSendingThreadParams pSendingThreadParams = NULL;
	PQueue senderQueue = NULL;
	
	//create sender queue
	int queueCreateResult = MessageQueue_Create(&senderQueue);
	if(SUCCESS_CODE != queueCreateResult)
	{
		printf("failed on creat senderQueue");
		return FAILURE_CODE;
	}
	//allocate  UI thread params struct and put values in its attributes
	int getUIParamsResult = Client_GetUIThreadParams(&pUIThreadParams, inputFile, senderQueue);
	if (SUCCESS_CODE != getUIParamsResult)
	{
		printf("failed on get UI thread params");
		return FAILURE_CODE;
	}
	//allocate  sending thread params struct and put values in its attributes
	int getsenderParamsResult = Client_GetSendingThreadParams(&pSendingThreadParams, serverConnection, senderQueue);
	if (SUCCESS_CODE != getUIParamsResult)
	{
		printf("failed on get UI thread params");
		return FAILURE_CODE;
	}

	PHANDLE clientThreadsArray = (PHANDLE)calloc(CLIENT_NUM_OF_THREADS, sizeof(clientThreadsArray[0]));//that gonna be a size of HANDLE // ASKME: the firs arg here was NUMBER_OF_CLIENT_THREADS from common.h even if thres 4 threads for client (including the mainthread) here im runing from that thread so it sounds wierd for me to insert handle of it here ehile im not creating it here, here its already runing
	if (NULL == clientThreadsArray)
	{
		printf("clientThreadsArray mem alloc failure");
		return FAILURE_CODE;
	}
	clientThreadsArray[UI_THREAD_INDEX] = BeginThreadExSimple(UIThread_Run, pUIThreadParams);//ASKME: i used MainThread_RunCalculationThreads as a reference. is the clientThreadsArray neccessary?
	if (NULL == clientThreadsArray[UI_THREAD_INDEX])
	{
		printf("failed on begining UserInterfaceThread thread");
		return FAILURE_CODE;
	}
	clientThreadsArray[SENDER_THREAD_INDEX] = BeginThreadExSimple(SendingThread_Run, pSendingThreadParams);
	if (NULL == clientThreadsArray[SENDER_THREAD_INDEX])
	{
		printf("failed on begining sending thread");
		return FAILURE_CODE;
	}
	*pClientThreadsArray = clientThreadsArray;
	return SUCCESS_CODE;
}