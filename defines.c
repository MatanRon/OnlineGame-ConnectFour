#include "Defines.h"

BOOL playerDisconnected;
volatile BOOL gameEnded;
SOCKET clientsSocketArr[NUM_OF_PLAYERS];
HANDLE clientsThreads[NUM_OF_PLAYERS];
HANDLE mutexOrderingResponses;
HANDLE mutexWriteToLog;
