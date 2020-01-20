#ifndef PROCESS_UTILS_H
#define PROCESS_UTILS_H

#include "Defines.h"

/*
# Description - CreateProcessSimple wraps CreateProcess(which creates a new process and its primary thread) so we wont be need to pass it all the arguments that CreateProcess expected to get and have the same values in avery call.
# Parameters - CommandLine: the line with the curr tests arguments ( including the prograt path itself)
			 - ProcessInfoPtr: A pointer to a process information structure that receives identification information about the new process.
# Returns - If the function succeeds, the return value is true. If the function fails, the return value is false.
*/
BOOL CreateProcessSimple(LPTSTR CommandLine, PROCESS_INFORMATION *ProcessInfoPtr);

/*
# Description - In a similar way to CreateProcessSimple, this function wraps _beginthreadex so we wont be need to pass it all the arguments that _beginthreadex expected to get and have the same
				values in avery call.
# Parameters - funcStartAddress: Lives Up to its name, holds the start address of the current program being tested
			 - pvFuncArgs: list of the program arguments.
# Returns - If succeeded, returns a handle to the newly created thread. Otherwise it returns NULL or -1 as an indication of a failure on start test thread.
*/
HANDLE BeginThreadExSimple(LPTHREAD_START_ROUTINE funcStartAddress, LPVOID pvFuncArgs);

#endif