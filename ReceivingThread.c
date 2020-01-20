
#include <stdio.h>

#include"ReceivingThread.h"
#include "Defines.h"
#include "PrintBoard.h"
#include "Loger.h"


#pragma warning(disable : 4996)



#define MESSAGE_TYPE_SEPARATOR ":\n"
#define MESSAGE_PARAMETER_SEPARATOR ";\n"

#define MESSAGE_TYPE_GAME_STARTED "GAME_STARTED"
#define MESSAGE_TYPE_TURN_SWITCH "TURN_SWITCH"
#define MESSAGE_TYPE_BOARD_VIEW "BOARD_VIEW"
#define MESSAGE_TYPE_CHAT "RECEIVE_MESSAGE"
#define MESSAGE_TYPE_GAME_ENDED "GAME_ENDED"
#define MESSAGE_TYPE_NEW_USER_ACCEPTED "NEW_USER_ACCEPTED"





DWORD WINAPI ReceivingThread_Run(LPVOID pParams)
{
	BOOL gameHasEnded = FALSE;
	PReceivingThreadParams pThreadParams = (PReceivingThreadParams)pParams;
	while (!gameHasEnded)
	{
		CHAR receivedMessage[MESSAGE_MAX_LENGTH] = { 0 };

		int receivingResult = recv(pThreadParams->serverConnection, receivedMessage, sizeof(receivedMessage), 0);
		if (SOCKET_ERROR == receivingResult)
		{
			CHAR stringToLoger[MESSAGE_MAX_LENGTH] = { 0 };
			int sprintfResult = sprintf(stringToLoger, "server disconnected. Exiting.");
			if ((-1) == sprintfResult)
			{
				printf("failed on make string to loger\n");
				exit(FAILURE_CODE);
			}
			Loger_WriteToLogAndPrint(stringToLoger, FALSE);
			exit(FAILURE_CODE);
		}
		
		CHAR stringToLogerBuf[MESSAGE_MAX_LENGTH] = { 0 };
		int tempSprintfResult = sprintf(stringToLogerBuf, "Received from server:%s\n", receivedMessage);
		if ((-1) == tempSprintfResult)
		{
			printf("failed on make string to loger\n");
			exit(FAILURE_CODE);
		}
		//Loger_WriteToLogAndPrint(stringToLogerBuf, TRUE);

		char *messageType = strtok(receivedMessage, MESSAGE_TYPE_SEPARATOR);
		//recognize message type and get params acordingly:
		if (0 == strcmp(messageType, MESSAGE_TYPE_NEW_USER_ACCEPTED))
		{
			// Nothing to do
		}
		if (0 == strcmp(messageType, MESSAGE_TYPE_GAME_STARTED))
		{
			printf("Game is on!\n");
		}
		else if (0 == strcmp(messageType, MESSAGE_TYPE_TURN_SWITCH))
		{
			char *userName = strtok(NULL, MESSAGE_PARAMETER_SEPARATOR);
			if (STRINGS_ARE_EQUAL(userName, pThreadParams->clientUsername))
			{
				BOOL success = SetEvent(pThreadParams->clientTurnEvent);
				if (!success)
				{
					printf("Setting client event failed on turn switch message\n");
					exit(FAILURE_CODE);
				}
			}

			CHAR stringToLoger[MESSAGE_MAX_LENGTH] = { 0 };
			int sprintfResult = sprintf(stringToLoger, "%s's turn\n",userName);
			if ((-1) == sprintfResult)
			{
				printf("failed on make string to loger\n");
				exit(FAILURE_CODE);
			}
			Loger_WriteToLogAndPrint(stringToLoger, FALSE);
		}
		
		else if (0 == strcmp(messageType, MESSAGE_TYPE_GAME_ENDED))
		{
			gameHasEnded = TRUE;
			char *userName = strtok(NULL, MESSAGE_PARAMETER_SEPARATOR);
			userName[strlen(userName)] = '\0';
			CHAR stringToLoger[MESSAGE_MAX_LENGTH] = { 0 };
			int sprintfResult = (-1);

			if (0 == strcmp(userName, "TIE\n"))
			{
				sprintfResult = sprintf(stringToLoger, "Game ended. Everybody wins!\n");
			}
			else
			{
				sprintfResult = sprintf(stringToLoger, "Game ended. The winner is %s!\n", userName);
			}

			if ((-1) == sprintfResult)
			{
				printf("failed on make string to loger\n");
				exit(FAILURE_CODE);
			}
			Loger_WriteToLogAndPrint(stringToLoger, FALSE);
		}

		else if (0 == strcmp(messageType, MESSAGE_TYPE_BOARD_VIEW))
		{
			int board[BOARD_HEIGHT][BOARD_WIDTH] = { 0 };
			for (size_t i = 0; i < BOARD_HEIGHT; i++)
			{
				for (size_t j = 0; j < BOARD_WIDTH; j++)
				{
					char *currentColorString = strtok(NULL, MESSAGE_PARAMETER_SEPARATOR); 
					board[BOARD_HEIGHT - i - 1][j] = strtol(currentColorString, NULL, BASE);
				}
			}

			HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
			if (NULL == consoleHandle)
			{
				Loger_WriteToLogAndPrint("Custom message: Getting console handle failed in board view\n", FALSE);
				exit(FAILURE_CODE);
			}
			PrintBoard(board, consoleHandle);
			printf("\n");
		}

		else if (0 == strcmp(messageType, MESSAGE_TYPE_CHAT))
		{
			char *userName = strtok(NULL, MESSAGE_PARAMETER_SEPARATOR);
			printf("%s:", userName);
			char *param = strtok(NULL, MESSAGE_PARAMETER_SEPARATOR);
			while (NULL != param)
			{
				printf("%s", param);
				param = strtok(NULL, MESSAGE_PARAMETER_SEPARATOR);
			}
			printf("\n");
		}
	}
	return SUCCESS_CODE;
}