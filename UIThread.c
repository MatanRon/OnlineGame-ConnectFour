#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>

#include "Defines.h"
#include "Client.h"
#include "UIThread.h"
#include "MessageQueue.h"
#include "Loger.h"
#pragma warning(disable : 4996)
#define MAX_COMMAND_LENGTH (10 + 1)
#define MAX_LINE_LENGTH (100 + 2) // +2 for \n\0
#define MAX_MESSAGE_LENGTH (210) // 2 characters for each message size(1 for actual char 1 for separator - at worst case)
#define ILLEGAL_COMMAND_PRINT ("Error: Illegal command\n")

SIZE_T UIThread_SkipWhitespace(CHAR *string, int offset, size_t size)
{
	SIZE_T index;
	for (index = offset; index < size; index++)
	{
		if (!isspace(string[index]))
		{
			break;
		}
	}
	return index;
}

int UIThread_ExecuteCommand(FILE *inputFile, PQueue senderQueue, CHAR *command, BOOL *gotExitCommand)
{
	*gotExitCommand = FALSE;
	CHAR message[MAX_MESSAGE_LENGTH] = { 0 };
	CHAR line[MAX_LINE_LENGTH];
	SIZE_T lineLength = strlen(line);
	
	if (stdin != inputFile)
	{
		Sleep(1000);
	}
	CHAR *file = fgets(line, sizeof(line), inputFile);
	if (NULL == file)
	{
		printf("Getting current line of user command failed");
		Loger_WriteToLogAndPrint("Custom message: Getting current line of user command failed\n", FALSE);
		exit(FAILURE_CODE);
	}

	if (0 == strcmp("play", command))
	{
		strcpy(message, "PLAY_REQUEST:"); 
		SIZE_T index = UIThread_SkipWhitespace(line, 0, lineLength);
		CHAR firstColumn = line[index];
		CHAR secondColumn = line[index + 1];
		
		// Check if we have extra characters
		index = UIThread_SkipWhitespace(line, index + 1, lineLength);
		if (lineLength == index)
		{
			printf("%s", ILLEGAL_COMMAND_PRINT);
			exit(FAILURE_CODE);
		}
		
		SIZE_T messageLength = strlen(message);
		message[messageLength] = firstColumn;
		message[messageLength + 1] = secondColumn;
		message[messageLength + 2] = '\0';
	}

	else if (0 == strcmp("message", command)) // message
	{
		strcpy(message, "SEND_MESSAGE:");
		SIZE_T currIndex = 1; // Skip first space
		SIZE_T messageCurrPosition = strlen(message);
		while (currIndex < lineLength)
		{	
			for (; currIndex < lineLength; currIndex++)
			{
				if (' ' == line[currIndex])
				{
					strcat(message, CHAT_MESSAGE_SEPARATOR_STRING); 
					messageCurrPosition += strlen(CHAT_MESSAGE_SEPARATOR_STRING);
				}

				else
				{
					message[messageCurrPosition++] = line[currIndex];
				}
			}
		}
		message[messageCurrPosition++] = '\0';
	}
	else if (0 == strcmp("exit", command)) // exit
	{
		*gotExitCommand = TRUE;
		
		return SUCCESS_CODE;
	}
	else
	{
		CHAR stringToLoger[MESSAGE_MAX_LENGTH] = { 0 };
		int sprintfResult = sprintf(stringToLoger, ILLEGAL_COMMAND_PRINT);
		if ((-1) == sprintfResult)
		{
			printf("failed on make string to loger\n");
			exit(FAILURE_CODE);
		}
		Loger_WriteToLogAndPrint(stringToLoger, FALSE);
		return SUCCESS_CODE;
	}
	
	CHAR *fullMessage = _strdup(message);
	if (NULL == fullMessage)
	{
		exit(FAILURE_CODE);
	}
	int enqueResult = MessageQueue_Enqueue(senderQueue, fullMessage);
	if (SUCCESS_CODE != enqueResult)
	{
		free(fullMessage);
		exit(FAILURE_CODE);
	}
	return SUCCESS_CODE;
}


DWORD WINAPI UIThread_Run(LPVOID lpParams) 
{
	PUIThreadParams pUIParams = (PUIThreadParams)lpParams;
	BOOL gotExitCommand = FALSE;
	while (!gotExitCommand)
	{
		CHAR currentCommand[MAX_COMMAND_LENGTH] = { 0 };
		int scanfResult = fscanf(pUIParams->inputFile, "%s", currentCommand);
		if (1 != scanfResult)
		{
			exit(FAILURE_CODE);
		}

		if (STRINGS_ARE_EQUAL(currentCommand, "play") && stdin != pUIParams->inputFile)
		{
			DWORD waitResult = WaitForSingleObject(pUIParams->clientTurnEvent, LONGEST_TURN_TIME_MS);
			switch (waitResult)
			{
			case WAIT_OBJECT_0:
				break;
			case WAIT_TIMEOUT:
				printf("Ui thread timeout when waiting for client's turn\n");
				exit(FAILURE_CODE);
			default:
				printf("Ui thread failed waiting for client's turn\n");
				exit(FAILURE_CODE);
			}
		}
		int executeResult = UIThread_ExecuteCommand(pUIParams->inputFile, pUIParams->senderQueue, currentCommand, &gotExitCommand);
		if (SUCCESS_CODE != executeResult)
		{
			exit(FAILURE_CODE);
		}
	}
	return SUCCESS_CODE;
}