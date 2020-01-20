#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winSock2.h>
#include <windows.h>
#include <wS2tcpip.h>
#include <stdio.h>

#include "ProcessUtils.h"
#include "Loger.h"
#include "Defines.h"
#include "Socket.h"
#include "ServerUtils.h"
#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable : 4996)
// Global Variables -----------------------------------




/*
Description :
Server_clientsThread recieves messages from client, interpts them and sends back responses
Parameters  :
no parameters
Returns     :
SUCCESS 
FAILURE
GAME_ENDED
*/
DWORD WINAPI Server_clientsThread(LPVOID lpParam);





/*
Description : 
serverMain_serverMain initializes a socket, listening it and accepting 2 connections while creating client thread for each of them.
Parameters  :
no parameters
Returns     :
SUCCESS
FAILURE
*/

DWORD WINAPI Server_Run()
{
	char errorLog[LENGTH_OF_SEND_STRING] = { 0 };
	numOfConnections = 0;
	SOCKADDR_IN service;

	SOCKET mainSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == mainSocket)
	{
		sprintf(errorLog, "Custom message: Error at socket(): %ld\n", WSAGetLastError());
		Loger_WriteToLogAndPrint(errorLog, FALSE);
		exit((DWORD)FAILURE_CODE);
	}
	unsigned long address = inet_addr(SERVER_ADDRESS_STR);
	if (INADDR_NONE == address)
	{
		sprintf(errorLog, "Custom message: The string - %s - cannot be converted into ip address\n", SERVER_ADDRESS_STR);
		Loger_WriteToLogAndPrint(errorLog, FALSE);
		exit((DWORD)FAILURE_CODE);
	}
	Socket_CreateSockadderServer(&service, &address, serverPort);
	int bindResult = bind(mainSocket, (SOCKADDR*)&service, sizeof(service));
	if (SOCKET_ERROR == bindResult)
	{
		sprintf(errorLog, "Custom message: Binding failed - %ld\n", WSAGetLastError());
		Loger_WriteToLogAndPrint(errorLog, FALSE);
		exit((DWORD)FAILURE_CODE);
	}
	int listenResult = listen(mainSocket, SOMAXCONN);
	if (SOCKET_ERROR == listenResult)
	{
		sprintf(errorLog, "Custom message: Failed listening on socket - %ld\n", WSAGetLastError());
		Loger_WriteToLogAndPrint(errorLog, FALSE);
		exit((DWORD)FAILURE_CODE);
	}
	while (TRUE)
	{
		SOCKET clientSocket = accept(mainSocket, NULL, NULL);
		if (INVALID_SOCKET == clientSocket)
		{
			sprintf(errorLog, "Custom message: Failed accepting client - %ld\n", WSAGetLastError());
			Loger_WriteToLogAndPrint(errorLog, FALSE);
			exit((DWORD)FAILURE_CODE);
		}
		if (2 == numOfConnections)
		{
			if (gameEnded)
			{
				ServerUtiles_InitBoard();
				numOfConnections = 0;
			}
			else
			{
				closesocket(clientSocket);
			}
		}
		else
		{
			playerDisconnected = FALSE;
			if (INVALID_SOCKET == clientSocket)
			{
				memset(errorLog, 0, LENGTH_OF_SEND_STRING);
				sprintf(errorLog, "Custom message: Failed accepting connection with client - %ld", WSAGetLastError());
				Loger_WriteToLogAndPrint(errorLog, FALSE);
				exit((DWORD)FAILURE_CODE);
			}
			if (SUCCESS_CODE == ServerUtils_NewUserRequest(&clientSocket));
			{
				numOfConnections++;
				clientsSocketArr[numOfConnections - 1] = clientSocket;
				clientsThreads[numOfConnections - 1] = BeginThreadExSimple(Server_clientsThread, (LPVOID)(numOfConnections));
			}
			if (2 == numOfConnections)
			{
				ServerUtiles_InitBoard();
				if (FAILURE_CODE == ServerUtils_GameStarted())
					exit((DWORD)FAILURE_CODE);
				turn = 1;
				if (FAILURE_CODE == ServerUtils_BoardView())
					exit((DWORD)FAILURE_CODE);
				if (FAILURE_CODE == ServerUtils_TurnSwitch())
					exit((DWORD)FAILURE_CODE);
			}
		}
	}
	return SUCCESS_CODE;
}


DWORD WINAPI Server_clientsThread(LPVOID lpParam)
{
	char *acceptedString = NULL;
	char stringToLog[LENGTH_OF_SEND_STRING] = { 0 };
	int playerNum = (int)lpParam;
	while (TRUE)
	{
		acceptedString = NULL;
		TransferResult_t receiveResult = ServerUtils_ReceiveString(&acceptedString, clientsSocketArr[playerNum - 1]);
		if (TRNS_DISCONNECTED == receiveResult)
		{
			if (gameEnded)
			{
				//game ended from other client
				free(acceptedString);
				return GAME_ENDED;
			}
			else if (playerDisconnected)
			{
				free(acceptedString);
				return SUCCESS_CODE;
			}
			//this client disconnected
			free(acceptedString);
			playerDisconnected = TRUE;
			sprintf(stringToLog, "Player disconnected. Ending communication");
			Loger_WriteToLogAndPrint(stringToLog, FALSE);
			ServerUtils_CloseConnections();
			return SUCCESS_CODE;
		}
		else if (TRNS_SUCCEEDED == receiveResult)
		{
			DWORD waitResult = WaitForSingleObject(mutexOrderingResponses, INFINITE);
			int decode = ServerUtils_DecodeMessage(acceptedString, playerNum, &clientsSocketArr[playerNum - 1]);
			waitResult = ReleaseMutex(mutexOrderingResponses);
			free(acceptedString);
			if (GAME_ENDED == decode)
				return GAME_ENDED;
			else if (SUCCESS_CODE != decode)
				exit(FAILURE_CODE);
		}
		else
		{
			return (DWORD)(FAILURE_CODE);
		}
	}
	return SUCCESS_CODE;
}

int Server_Init()
{
	numOfConnections = 0;

	ServerUtiles_InitBoard();

	mutexOrderingResponses = CreateMutex(NULL, FALSE, NULL);
	if (NULL == mutexOrderingResponses)
	{
		printf("Failed creating mutex\n");
		exit(FAILURE_CODE);
	}

	Server_Run();
	return SUCCESS_CODE;
}