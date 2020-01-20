#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H
#include <windows.h>
typedef struct _Queue *PQueue;

int MessageQueue_Create(PQueue *pPQueue)
void MessageQueue_Cleanup(PQueue pQueue);

void MessageQueue_Enqueue(PQueue pQueue, CHAR *elementToInsert);
int MessageQueue_Dequeue(PQueue pQueue, CHAR **message/*out param*/);



#endif
