#include "Loger.h"
#include "Defines.h"
#pragma warning(disable : 4996)
//////////global variables-----------
 

void Loger_WriteToLogAndPrint(char *input, BOOL onlyToLog )
{
	DWORD waitResult = WaitForSingleObject(mutexWriteToLog, INFINITE);
	if (WAIT_OBJECT_0 != waitResult)
	{
		printf("Failed to obtain mutex for writing errNo: %d\n", GetLastError());
		exit(FAILURE_CODE);
	}
	pLogerFile = fopen(pLogFilePath, "a+");
	if (NULL == pLogerFile)
	{
		printf("Failed to open log file for writing errNo : %d\n", GetLastError());
		exit(FAILURE_CODE);
	}

	fprintf(pLogerFile, input);//print to log file
	if (!onlyToLog)
	{
		printf("%s", input);
	}
	fclose(pLogerFile);
	ReleaseMutex(mutexWriteToLog);
}