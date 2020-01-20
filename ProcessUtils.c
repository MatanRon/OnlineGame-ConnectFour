#include <process.h>

#include "ProcessUtils.h"

/*
# Description - CreateProcessSimple wraps CreateProcess(which creates a new process and its primary thread) so we wont be need to pass it all the arguments that CreateProcess expected to get and have the same values in avery call. 
# Parameters - CommandLine: the line with the curr tests arguments ( including the prograt path itself) 
			 - ProcessInfoPtr: A pointer to a process information structure that receives identification information about the new process.
# Returns - If the function succeeds, the return value is true. If the function fails, the return value is false.
*/
BOOL CreateProcessSimple(LPTSTR CommandLine, PROCESS_INFORMATION *ProcessInfoPtr) {
	STARTUPINFO	startinfo = { sizeof(STARTUPINFO), NULL, 0 }; /* <ISP> here we */
															  /* initialize a "Neutral" STARTUPINFO variable. Supplying this to */
															  /* CreateProcess() means we have no special interest in this parameter. */
															  /* This is equivalent to what we are doing by supplying NULL to most other */
															  /* parameters of CreateProcess(). */

	return CreateProcess(NULL, /*  No module name (use command line). */
		CommandLine,			/*  Command line. */
		NULL,					/*  Process handle not inheritable. */
		NULL,					/*  Thread handle not inheritable. */
		FALSE,					/*  Set handle inheritance to FALSE. */
		NORMAL_PRIORITY_CLASS,	/*  creation/priority flags. */
		NULL,					/*  Use parent's environment block. */
		NULL,					/*  Use parent's starting directory. */
		&startinfo,				/*  Pointer to STARTUPINFO structure. */
		ProcessInfoPtr			/*  Pointer to PROCESS_INFORMATION structure. */
	);
}

/*
# Description - In a similar way to CreateProcessSimple, this function wraps _beginthreadex so we wont be need to pass it all the arguments that _beginthreadex expected to get and have the same 
				values in avery call.
# Parameters - funcStartAddress: Lives Up to its name, holds the start address of the current program being tested
			 - pvFuncArgs: list of the program arguments.
# Returns - If succeeded, returns a handle to the newly created thread. Otherwise it returns NULL or -1 as an indication of a failure on start test thread.
*/
HANDLE BeginThreadExSimple(LPTHREAD_START_ROUTINE funcStartAddress, LPVOID pvFuncArgs) {
	return (HANDLE)_beginthreadex(NULL, 0, (_beginthreadex_proc_type)funcStartAddress, pvFuncArgs, 0, NULL);
}