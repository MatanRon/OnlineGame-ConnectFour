#ifndef PROCESS_UTILS_H
#define PROCESS_UTILS_H

#include <windows.h>
#include "Defines.h"

BOOL CreateProcessSimple(LPTSTR CommandLine, PROCESS_INFORMATION *ProcessInfoPtr);
HANDLE BeginThreadExSimple(LPTHREAD_START_ROUTINE funcStartAddress, LPWORD pTreadId, LPVOID pvFuncArgs);

#endif