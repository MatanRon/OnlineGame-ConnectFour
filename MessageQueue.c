#include <stdlib.h>
#include <stdio.h>
#include "Defines.h"

#include"MessageQueue.h"
#include "Loger.h"


typedef struct _Node
{
	char *message;
	struct _Node *next;
} Node, *PNode;

typedef struct _Queue
{
	PNode head;
	HANDLE mutex;
	HANDLE notEmptyEvent;
} Queue, *PQueue;

int MessageQueue_CreateNode(char *message, PNode nextNode, PNode *pPNewNode/*out parameter*/) 
{
	PNode newNode = NULL;
	newNode = (PNode)malloc(sizeof(*newNode));
	if (NULL == newNode)
	{
		Loger_WriteToLogAndPrint("Custom message: memory alocation for message faild\n", FALSE);
		exit(FAILURE_CODE);
	}
	newNode->message = message;
	newNode->next = nextNode;

	*pPNewNode = newNode;
	return SUCCESS_CODE;

}

void MessageQueue_Cleanup(PQueue pQueue) 
{

	PNode curreNodeP = pQueue->head;
	PNode tempNodeP = NULL;
	while (curreNodeP != NULL)
	{
		tempNodeP = curreNodeP;
		curreNodeP = curreNodeP->next;
		free(tempNodeP->message);
		free(tempNodeP);
	}
	pQueue->head = NULL;
	(VOID)ReleaseMutex(pQueue->mutex);
	(VOID)CloseHandle(pQueue->mutex);
	(VOID)CloseHandle(pQueue->notEmptyEvent);
	free(pQueue);
}

int MessageQueue_Create(PQueue *pPQueue) 
{
	PQueue newQueue = NULL;
	newQueue = (PQueue)malloc(sizeof(*newQueue));
	if (NULL == newQueue)
	{
		Loger_WriteToLogAndPrint("Custom message: memory alocation for queue faild\n", FALSE);
		exit(FAILURE_CODE);
	}

	newQueue->head = NULL;

	newQueue->mutex = CreateMutex(NULL, FALSE, NULL);
	if (NULL == newQueue->mutex)
	{
		Loger_WriteToLogAndPrint("Custom message: Queue mutex failure\n", FALSE);
		exit(FAILURE_CODE);
	}

	newQueue->notEmptyEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (NULL == newQueue->notEmptyEvent )
	{
		Loger_WriteToLogAndPrint("Custom message: failed on creating Event\n", FALSE);
		exit(FAILURE_CODE);
	}
	*pPQueue = newQueue;
	return SUCCESS_CODE;
}


int MessageQueue_Enqueue(PQueue pQueue, CHAR *elementToInsert) //LinkedList_AddToStart
{
	
	DWORD waitResult = WaitForSingleObject(pQueue->mutex, LONGEST_MUTEX_WAITING_TIME_MS);
	switch (waitResult)
	{
	case WAIT_OBJECT_0:

		break;
	case WAIT_TIMEOUT:
		Loger_WriteToLogAndPrint("Custom message: waiting to long for enqueue mutex\n", FALSE);
		exit(FAILURE_CODE);
	case WAIT_ABANDONED:
		exit(FAILURE_CODE);
	default:
		Loger_WriteToLogAndPrint("Custom message: failed on get mutex for enqueue\n", FALSE);
		exit(FAILURE_CODE);
	}

	//assuming happy flow here i.e. mutex is locked by me
	PNode pNewNode = NULL;
	int createNodeResult = MessageQueue_CreateNode(elementToInsert, NULL, &pNewNode);
	if (SUCCESS_CODE != createNodeResult)
	{
		Loger_WriteToLogAndPrint("Custom message: failed on creating new Node\n", FALSE);
		exit(FAILURE_CODE);
	}

	if (NULL == pQueue->head)
	{
		pQueue->head = pNewNode;
	}

	else
	{
		PNode tailOfList = pQueue->head;
		while (NULL != tailOfList->next)
		{
			tailOfList = tailOfList->next;
		}

		tailOfList->next = pNewNode;
	}

	BOOL success = SetEvent(pQueue->notEmptyEvent);
	BOOL releaseResult = FALSE;
	if (!success)
	{
		MessageQueue_Cleanup(pQueue);
		Loger_WriteToLogAndPrint("Custom message: failed releas event\n", FALSE);
		exit(FAILURE_CODE);
	}
	releaseResult = ReleaseMutex(pQueue->mutex);
	if (!releaseResult)
	{
		Loger_WriteToLogAndPrint("Custom message: Release mutex failed after enqueue. \n", FALSE);
		exit(FAILURE_CODE);
	}
	return SUCCESS_CODE;
}

// This works because we only use 1 consumer(only 1 thread does MessageQueue_Dequeue) - no sync problems.
int MessageQueue_Dequeue(PQueue pQueue, CHAR **message)
{
	DWORD eventWaitResult = WaitForSingleObject(pQueue->notEmptyEvent, INFINITE);
	switch (eventWaitResult)
	{
	case WAIT_OBJECT_0:
		break;
	case WAIT_TIMEOUT:
		Loger_WriteToLogAndPrint("Custom message: waiting to long for enqueue event\n", FALSE);
		exit(FAILURE_CODE);
	case WAIT_ABANDONED:
		Loger_WriteToLogAndPrint("Custom message: wait abandoned enque event\n", FALSE);
		exit(FAILURE_CODE);
	default:
		Loger_WriteToLogAndPrint("Custom message: failed on get event for enqueue\n", FALSE);
		exit(FAILURE_CODE);
	}

	DWORD mutexWaitResult = WaitForSingleObject(pQueue->mutex, LONGEST_MUTEX_WAITING_TIME_MS);
	switch (mutexWaitResult)
	{
	case WAIT_OBJECT_0:
		break;
	case WAIT_TIMEOUT:
		Loger_WriteToLogAndPrint("Custom message: waiting to long for enqueue mutex\n", FALSE);
		exit(FAILURE_CODE);
	case WAIT_ABANDONED:
		Loger_WriteToLogAndPrint("Custom message: wait abandoned enque mutex\n", FALSE);
		exit(FAILURE_CODE);
	default:
		Loger_WriteToLogAndPrint("Custom message: failed on get mutex for enqueue\n", FALSE);
		exit(FAILURE_CODE);
	}

	PNode head = pQueue->head;
	pQueue->head = head->next;

	*message = head->message;
	free(head);
	BOOL releaseResult = FALSE;
	if (NULL == pQueue->head)
	{
		BOOL success = ResetEvent(pQueue->notEmptyEvent);
		if (!success)
		{
			releaseResult = ReleaseMutex(pQueue->mutex);
			if (!releaseResult)
			{
				Loger_WriteToLogAndPrint("Custom message: Release mutex failed after dequeue\n", FALSE);
			}
			exit(FAILURE_CODE);
		}
	}
	releaseResult = ReleaseMutex(pQueue->mutex);
	if (!releaseResult)
	{
		Loger_WriteToLogAndPrint("Custom message: Release mutex failed after dequeue\n", FALSE);
		exit(FAILURE_CODE);
	}
	return SUCCESS_CODE;
}


//int main()
//{
//	char *a = "aaa";
//	char *b = "bbb";
//	char *c = "ccc";
//	char *message = NULL;
//	PQueue pQ = NULL;
//	int retVal;
//	retVal = MessageQueue_Create(&pQ);
//	retVal = MessageQueue_Enqueue(pQ, a);
//	retVal = MessageQueue_Enqueue(pQ, b);
//	retVal = MessageQueue_Enqueue(pQ, c);
//	retVal = MessageQueue_Dequeue(pQ, &message);
//	retVal = MessageQueue_Dequeue(pQ, &message);
//	retVal = MessageQueue_Dequeue(pQ, &message);
//
//	return 1;
//
//}