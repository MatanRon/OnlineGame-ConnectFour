/************************************************************************
*	PROJECT:			Connect Four  									*
* 																		*
************************************************************************/

/*!
 * 
 * @Author	Matan Ron
 * @Date	12/01/19
 *
 * @Brief:
 *	In this project an online version of the game "Connect Four" is implemented. 
 *  Chat between players is also included :)
 *
 *	The program has two running modes (only one at a time):
 *	Server Mode - Manages the game. Receives incoming communication from client applications, 
 *	calculates board status and distributes messages between customers.
 *	
 *  Client mode - the user interface for the game (connects to the server using the TCP protocol). 
 *  Commands are received from the player, and sent to the server. 
 *	Also, the software receives updates from the server, and displays it to the player.
 *
 */

#define WIN32_LEAN_AND_MEAN

#include <ws2tcpip.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "Defines.h"
#include "Client.h"
#include "Server.h"
#include"Loger.h"

#pragma warning(disable : 4996)

typedef enum
{
	MAIN_ARGS_SERVER_MODE_PROGRAM_NAME = 0,
	MAIN_ARGS_SERVER_MODE_PROGRAM_MODE, //always shuld be server
	MAIN_ARGS_SERVER_MODE_LOG_FILE_PATH,
	MAIN_ARGS_SERVER_MODE_SERVER_PORT_NUMBER,


	MAIN_ARGS_SERVER_MODE_COUNT
} MainArgsOnServerMode;

typedef enum
{
	MAIN_ARGS_CLIENT_MODE_PROGRAM_NAME = 0,
	MAIN_ARGS_CLIENT_MODE_PROGRAM_MODE, //always shuld be client
	MAIN_ARGS_CLIENT_MODE_LOG_FILE_PATH,
	MAIN_ARGS_CLIENT_MODE_SERVER_PORT_NUMBER,
	MAIN_ARGS_CLIENT_MODE_INPUT_MODE,
	MAIN_ARGS_CLIENT_MODE_INPUT_FILE_PATH,

	MAIN_ARGS_CLIENT_MODE_COUNT
} MainArgsOnClientMode;

#define MAIN_ARGS_HUMAN_MODE_COUNT (MAIN_ARGS_CLIENT_MODE_INPUT_MODE + 1)

void main_Cleanup(FILE *inputFile, BOOL isHuman)
{
	if (!isHuman)
	{
		// Best effort
		(void)fclose(inputFile);
	}

}

int main_ArgumentsParseAndCheck(int argc, char *argv[], FILE **inputFile/*outParam*/, BOOL *isHuman/*outParam*/, BOOL *isClientMode/*outParam*/, unsigned short *serverPortAddress/*outParam*/)
{
	BOOL wrongInput = TRUE;

	mutexWriteToLog = CreateMutex(NULL, FALSE, NULL);
	if (NULL == mutexWriteToLog)
	{
		printf("Failed creating mutex\n");
		exit(FAILURE_CODE);
	}

	if ((argc != MAIN_ARGS_SERVER_MODE_COUNT) && (argc != MAIN_ARGS_CLIENT_MODE_COUNT) && (argc != MAIN_ARGS_HUMAN_MODE_COUNT))
	{
		
		//Loger_WriteToLogAndPrint("Custom message: wrong program arguments input \n", FALSE);
		exit(FAILURE_CODE);
	}

	if (0 == strcmp("server", argv[MAIN_ARGS_SERVER_MODE_PROGRAM_MODE]))
	{
		if (argc != MAIN_ARGS_SERVER_MODE_COUNT)
		{
			//Loger_WriteToLogAndPrint("Custom message: Wrong Server Mode arguments input \n", FALSE);
			exit(FAILURE_CODE);
		}
		wrongInput = FALSE;
		*isClientMode = FALSE;
	}

	if (0 == strcmp("client", argv[MAIN_ARGS_CLIENT_MODE_PROGRAM_MODE]))
	{
		if (argc == MAIN_ARGS_CLIENT_MODE_COUNT)
		{
			if (0 == strcmp("file", argv[MAIN_ARGS_CLIENT_MODE_INPUT_MODE]))
			{
				wrongInput = FALSE;
				*isClientMode = TRUE;
				*inputFile = fopen(argv[MAIN_ARGS_CLIENT_MODE_INPUT_FILE_PATH], "r");
				if (NULL == *inputFile)
				{
					printf("failed open input file");
					exit(FAILURE_CODE);
				}
			}
		}
		else if (argc == (MAIN_ARGS_HUMAN_MODE_COUNT))
		{
			if (0 == strcmp("human", argv[MAIN_ARGS_CLIENT_MODE_INPUT_MODE]))
			{
				*isHuman = TRUE;
				wrongInput = FALSE;
				*inputFile = stdin;
			}
		}

		*isClientMode = TRUE;
	}

	*serverPortAddress = (unsigned short)strtol(argv[MAIN_ARGS_CLIENT_MODE_SERVER_PORT_NUMBER], NULL, BASE);
	if (0 == *serverPortAddress)
	{
		//Loger_WriteToLogAndPrint("Custom message: server port address error\n", FALSE);
		exit(FAILURE_CODE);
	}
	

	if (wrongInput)
	{
		//Loger_WriteToLogAndPrint("Custom message: wrong program arguments input\n", FALSE);
		exit(FAILURE_CODE);
	}

	return SUCCESS_CODE;
}

int main_InitWsa(WORD *wVersionRequested)
{
	WSADATA wsaData = { 0 };

	*wVersionRequested = MAKEWORD(2, 2);
	int returnValue;
	returnValue = WSAStartup(*wVersionRequested, &wsaData);
	if (returnValue != 0)
	{
		/* Tell the user that we could not find a usable */
		/* Winsock DLL.                                  */
		//Loger_WriteToLogAndPrint("Custom message: WSAStartup failed\n", FALSE);
		exit(FAILURE_CODE);
	}

	if (LOBYTE((wsaData).wVersion) != 2 || HIBYTE((wsaData).wVersion) != 2) {
		/* Tell the user that we could not find a usable */
		/* WinSock DLL.                                  */
		//Loger_WriteToLogAndPrint("Custom message: Could not find a usable version of Winsock.dll\n", FALSE);
		exit(FAILURE_CODE);
	}
	else
	{
		printf("The Winsock 2.2 dll was found okay\n");
	}
	return SUCCESS_CODE;
}

// We use functions that return int: SUCCESS_CODE / FAILURE_CODE, but always exit on failure.
// This allows changing the failure handling in the future easy(return insted of exit)
int main(int argc, char *argv[])
{
	BOOL isClientMode = FALSE;// TRUE for client mode. FALSE for server mode.
	
	unsigned short serverPortAddress = 0;
	FILE *inputFile = NULL;
	BOOL isHuman = FALSE;
	
	// init wsa arguments
	WORD wVersionRequested = 0;

	main_ArgumentsParseAndCheck(argc, argv, &inputFile, &isHuman, &isClientMode, &serverPortAddress);
	
	main_InitWsa(&wVersionRequested);
	


	if (isClientMode)
	{
		strcpy(pLogFilePath, argv[MAIN_ARGS_CLIENT_MODE_LOG_FILE_PATH]);
		pLogerFile = fopen(pLogFilePath, "w");
		if (NULL == pLogerFile)
		{
			printf("log faile open failed\n");
			exit(FAILURE_CODE);

		}

		int clientRunReturnValue = Client_Run(serverPortAddress, inputFile, isHuman);
		if (SUCCESS_CODE != clientRunReturnValue)
		{
			Loger_WriteToLogAndPrint("Custom message: client_Run Failure\n", FALSE);
			exit(FAILURE_CODE);
		}
	}
	else//Server mode
	{
		serverPort = (unsigned short)strtol(argv[MAIN_ARGS_SERVER_MODE_SERVER_PORT_NUMBER], NULL, BASE);
		strcpy(pLogFilePath, argv[MAIN_ARGS_SERVER_MODE_LOG_FILE_PATH]);
		pLogerFile = fopen(pLogFilePath, "w");
		if (NULL == pLogerFile)
		{
			printf("log faile open failed\n");
			exit(FAILURE_CODE);

		}
		Server_Init();
	}

	WSACleanup();
	main_Cleanup(inputFile, isHuman);
	exit(SUCCESS_CODE);
}
