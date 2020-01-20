#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H
#include <windows.h>
typedef struct _Queue *PQueue;  

/*
Description - MessageQueue_Create creates new queue for massages
Parameters  - pPQueue for the queue
returns     : success_code if all good, faiure_code else
*/
int MessageQueue_Create(PQueue *pPQueue);

/*
Description - MessageQueue_Cleanup cleans the queue
Parameters  - pQueue - the queue
returns     : none(void function)
*/
void MessageQueue_Cleanup(PQueue pQueue);

/*
Description - MessageQueue_Enqueue enqueue message
Parameters  - pQueue - the queue, elementToInsert what element to insert
returns     : success_code if all good, failure_code else
*/
int MessageQueue_Enqueue(PQueue pQueue, CHAR *elementToInsert);

/*
Description - MessageQueue_Dequeue dequeues message
Parameters  - pQueue - the queue, message - the massage
returns     : success_code if all good, failure_code else
*/
int MessageQueue_Dequeue(PQueue pQueue, CHAR **message);


#endif
