#include "ServerUtils.h"
#pragma warning(disable : 4996)

int IsUsernameFree(char users[][LENGTH_OF_USERNAME], char *username, int numberOfUsers)
{
	if (!username || !users)
	{
		Loger_WriteToLogAndPrint("Custom message: Not valid arguments to IsUsernameFree()", FALSE);
		return FAILURE_CODE;
	}
	for (int i = 0; i < numberOfUsers; i++)
	{
		if (STRINGS_ARE_EQUAL(users[i], username))
		{
			return FAILURE_CODE;
		}
	}
	return SUCCESS_CODE;
}


int ServerUtils_NewUserRequest(SOCKET *clientSocket)
{
	
	char *acceptedString = NULL;
	char returnMessage[LENGTH_OF_SEND_STRING] = "NEW_USER_ACCEPTED:";
	TransferResult_t receivedStr = ServerUtils_ReceiveString(&acceptedString, *clientSocket);
	printf("%s\n", acceptedString);
	if (TRNS_SUCCEEDED != receivedStr)
	{
		Loger_WriteToLogAndPrint("Custom message: Failed getting new user request", FALSE);
		return FAILURE_CODE; 
	}
	//get username
	if (!(strstr(acceptedString, "NEW_USER_REQUEST") == acceptedString))
	{
		Loger_WriteToLogAndPrint("Custom message: NEED TO SEND NEW_USER_REQUEST", FALSE);
		free(acceptedString);
		return FAILURE_CODE; //or exit?!
	}
	char username[LENGTH_OF_SEND_STRING] = { 0 };
	char *usernameStart = strchr(acceptedString, ':');
	usernameStart = strtok(usernameStart, "\n");
	strcpy_s(username, LENGTH_OF_SEND_STRING, usernameStart + 1);
	//check if username available
	if (IsUsernameFree(usernames, username, numOfConnections) != SUCCESS_CODE)
	{
		Loger_WriteToLogAndPrint("Username is taken!", TRUE);
		ServerUtils_SendString("NEW_USER_DECLINED", *clientSocket);
		free(acceptedString);
		return FAILURE_CODE;
	}
	if (0 == numOfConnections)
	{
		strcat(returnMessage, "1");
		strcpy(usernames[0], username);
	}
	else
	{
		strcat(returnMessage, "2");
		strcpy(usernames[1], username);
	}
	TransferResult_t sendResult = ServerUtils_SendString(returnMessage, *clientSocket);
	if (TRNS_FAILED == sendResult)
	{
		exit(FAILURE_CODE);
	}
	free(acceptedString);
	return SUCCESS_CODE;
}


TransferResult_t ServerUtils_ReceiveBuffer(char *pOutputBuffer, SOCKET socket)
{
	CHAR stopCondition = '\0';
	BOOL allStringReceived = FALSE;
	char *pCurrentPlace = pOutputBuffer;
	while (FALSE == allStringReceived)
	{
		int transferedBytes = recv(socket, pCurrentPlace, 1, 0);

		
		switch (transferedBytes)
		{
		case SOCKET_ERROR:
			return TRNS_FAILED;
		case 0:
			return TRNS_DISCONNECTED;
		default:
			
			if (stopCondition == pCurrentPlace[0])
			{
				allStringReceived = TRUE;
				break;
			}
		}
		pCurrentPlace += transferedBytes;
	}
	pCurrentPlace[0] = '\0';
	return TRNS_SUCCEEDED;
}


TransferResult_t ServerUtils_ReceiveString(char **ppOutputString, SOCKET socket)
{
	if ((NULL == ppOutputString) || (NULL != *ppOutputString))
	{
		Loger_WriteToLogAndPrint("Custom message:Not a valid input\n", FALSE);
		return TRNS_FAILED;
	}
	//allocating memory
	char *buffer = (char*)calloc(LENGTH_OF_SEND_STRING, sizeof(char));
	if (NULL == buffer)
	{
		Loger_WriteToLogAndPrint("Custom message: Cant allocate space for buffer\n", FALSE);
		return TRNS_FAILED;
	}
	

	//receive data into buffer
	TransferResult_t receiveResult = ServerUtils_ReceiveBuffer(buffer, socket);
	if (TRNS_SUCCEEDED == receiveResult)
		*ppOutputString = buffer;
	else
		free(buffer);
	return receiveResult;
}
	
TransferResult_t ServerUtils_SendBuffer(const char *buffer, int bytesToSend, SOCKET socket)
{
	const char *pCurPlace = buffer;
	int remainingBytesToSend = bytesToSend;

	while (remainingBytesToSend > 0)
	{
		int bytesTransfered = send(socket, pCurPlace, remainingBytesToSend, 0);
		if (SOCKET_ERROR == bytesTransfered)
		{
			return TRNS_FAILED;
		}
		remainingBytesToSend -= bytesTransfered;
		pCurPlace += bytesTransfered;
	}
	return TRNS_SUCCEEDED;
}

TransferResult_t ServerUtils_SendString(char *string, SOCKET socket)
{
	int totalStringSizeInBytes = (int)strlen(string);
	TransferResult_t sendResult = ServerUtils_SendBuffer((const char*)string, totalStringSizeInBytes, socket);
	return sendResult;
}

void ServerUtils_CloseConnections()
{
	char *acceptedString = NULL;
	shutdown(clientsSocketArr[0], SD_SEND);
	while (ServerUtils_ReceiveString(&acceptedString, clientsSocketArr[0]) != TRNS_DISCONNECTED)
	{
		free(acceptedString);
		acceptedString = NULL;
	}
	CloseHandle(clientsThreads[0]);
	closesocket(clientsSocketArr[0]);
	free(acceptedString);
	acceptedString = NULL;

	if (2 == numOfConnections)
	{
		shutdown(clientsSocketArr[1], SD_SEND);
		while (ServerUtils_ReceiveString(&acceptedString, clientsSocketArr[1]) != TRNS_DISCONNECTED)
		{
			free(acceptedString);
			acceptedString = NULL;
		}
		free(acceptedString);
		closesocket(clientsSocketArr[1]);
		CloseHandle(clientsThreads[1]);
	}
	numOfConnections = 0;
}

int ServerUtils_DecodeMessage(char *inputStringFromUser, int playerNum, SOCKET *clientSocket)
{
	char stringToLog[LENGTH_OF_SEND_STRING] = { 0 };
	char *colChar;
	TransferResult_t sendResult;
	int playerColor;
	if (1 == playerNum)
		playerColor = RED_PLAYER;
	else
		playerColor = YELLOW_PLAYER;
	if (strstr(inputStringFromUser, "PLAY_REQUEST") == inputStringFromUser)
	{
		if (0 == turn)
		{
			strcpy(stringToLog, "PLAY_DECLINED:");
			strcat(stringToLog, "Game has not started\n");
			sendResult = ServerUtils_SendString(stringToLog, *clientSocket);
		}
		else if (turn != playerNum)
		{
			strcpy(stringToLog, "PLAY_DECLINED:");
			strcat(stringToLog, "Not your turn\n");
			sendResult = ServerUtils_SendString(stringToLog, *clientSocket);
		}
		else
		{
			colChar = strtok(inputStringFromUser, "\0");
			colChar = strtok(colChar, "PLAY_REQUEST:");
			int col = atoi(colChar);
			int row = BOARD_HEIGHT - 1;
			if (col > 6 || col < 0 || arr[row][col] != BLACK)
			{
				strcpy(stringToLog, "PLAY_DECLINED:");
				strcat(stringToLog, "Illegal move\n");
				sendResult = ServerUtils_SendString(stringToLog, *clientSocket);
			}
			else
			{
				int i = 0;
				for (i = 0; i < BOARD_HEIGHT; i++)
					if (arr[i][col] == BLACK)
					{
						arr[i][col] = playerColor;
						break;
					}
				turn = 3 - turn;
				strcpy(stringToLog, "PLAY_ACCEPTED\n");
				sendResult = ServerUtils_SendString(stringToLog, *clientSocket);
				if (TRNS_FAILED == sendResult || TRNS_DISCONNECTED == sendResult)
				{
					return FAILURE_CODE;
				}
				int checkForWin = AreFourConnected(playerColor);
				if (SUCCESS_CODE == checkForWin)
				{
					GameEnded(playerNum, FALSE);
					return GAME_ENDED;
				}
				if (ItsATie())
				{
					GameEnded(playerNum, TRUE);
					return (GAME_ENDED);
				}
				else
				{
					ServerUtils_BoardView();
					ServerUtils_TurnSwitch();
				}
			}
		}
	}
	else if (strstr(inputStringFromUser, "SEND_MESSAGE") == inputStringFromUser)
	{
		char *string = strtok(inputStringFromUser, "SEND_MESSAGE:");
		
		strcpy(stringToLog, "RECEIVE_MESSAGE:");
		strcat(stringToLog, usernames[playerNum - 1]);
		strcat(stringToLog, ";");
		strcat(stringToLog, string);
		
		sendResult = ServerUtils_SendString(stringToLog, clientsSocketArr[2 - playerNum]);
		if (TRNS_FAILED == sendResult)
			return FAILURE_CODE;
		else
			return SUCCESS_CODE;

	}
	return SUCCESS_CODE;
}
int AreFourConnected(int playerColor)
{
	for (int row = 0; row < BOARD_HEIGHT; row++)
	{
		for (int col = 0; col < BOARD_WIDTH - 3; col++)
		{
			if (playerColor == arr[row][col] && playerColor == arr[row][col + 1] && playerColor == arr[row][col + 2] && playerColor == arr[row][col + 3])
				return SUCCESS_CODE;
		}
	}
	for (int row = 0; row < BOARD_HEIGHT - 3; row++)
	{
		for (int col = 0; col < BOARD_WIDTH; col++)
		{
			if (playerColor == arr[row][col] && playerColor == arr[row + 1][col] && playerColor == arr[row + 2][col] && playerColor == arr[row + 3][col])
				return SUCCESS_CODE;
		}
	}
	for (int row = 0; row < BOARD_HEIGHT - 3; row++)
	{
		for (int col = 0; col < BOARD_WIDTH - 3; col++)
		{
			if (playerColor == arr[row][col] && playerColor == arr[row + 1][col + 1] && playerColor == arr[row + 2][col + 2] && playerColor == arr[row + 3][col + 3])
				return SUCCESS_CODE;
		}
	}
	for (int row = 0; row < BOARD_HEIGHT - 3; row++)
	{
		for (int col = 3; col < BOARD_WIDTH; col++)
		{
			if (playerColor == arr[row][col] && playerColor == arr[row + 1][col - 1] && playerColor == arr[row + 2][col - 2] && playerColor == arr[row + 3][col - 3])
				return SUCCESS_CODE;
		}
	}
	return FAILURE_CODE;
}

int GameEnded(int playerNum, BOOL tie)
{
	char stringToLog[LENGTH_OF_SEND_STRING] = { 0 };
	char *acceptedString = NULL;
	gameEnded = TRUE;
	strcpy(stringToLog, "GAME_ENDED:");
	if (tie)
	{
		strcat(stringToLog, "TIE\n");
	}
	else
	{
		strcat(stringToLog, usernames[playerNum - 1]);
		strcat(stringToLog, "\n");
	}
	TransferResult_t sendResult1 = ServerUtils_SendString(stringToLog, clientsSocketArr[0]);
	TransferResult_t sendResult2 = ServerUtils_SendString(stringToLog, clientsSocketArr[1]);
	if (TRNS_DISCONNECTED == sendResult1 || TRNS_DISCONNECTED == sendResult2)
	{
		return FAILURE_CODE;

	}
	shutdown(clientsSocketArr[0], SD_BOTH);
	shutdown(clientsSocketArr[1], SD_BOTH);
	//while (TRNS_DISCONNECTED != ServerUtils_ReceiveString(&acceptedString, clientsSocketArr[0]))
	//{
	//	free(acceptedString);
	//	acceptedString = NULL;
	//}
	//while (TRNS_DISCONNECTED != ServerUtils_ReceiveString(&acceptedString, clientsSocketArr[1]))
	//{
	//	free(acceptedString);
	//	acceptedString = NULL;
	//}
	closesocket(clientsSocketArr[0]);
	closesocket(clientsSocketArr[1]);
	CloseHandle(clientsThreads[0]);
	CloseHandle(clientsThreads[1]);
	printf("close\n");
	numOfConnections = 0;
	turn = 0;
	ServerUtiles_InitBoard();

	return SUCCESS_CODE;
}
void ServerUtiles_InitBoard()
{
	for (int i = 0; i < BOARD_HEIGHT; i++)
	{
		for (int j = 0; j < BOARD_WIDTH; j++)
		{
			arr[i][j] = BLACK;
		}
	}
}
BOOL ItsATie()
{
	int count = 0;
	for (int i = 0; i < BOARD_HEIGHT; i++)
	{
		for (int j = 0; j < BOARD_WIDTH; j++)
		{
			if (arr[i][j] != BLACK)
			{
				count++;
			}
		}
	}
	if (count == 42)
		return TRUE;
	return FALSE;
}
int ServerUtils_BoardView()
{
	char stringToLog[LENGTH_OF_SEND_STRING] = { 0 };
	char stringToHelp[LENGTH_OF_SEND_STRING] = { 0 };
	strcpy(stringToLog, "BOARD_VIEW:");
	for (int i = 0; i < BOARD_HEIGHT; i++)
	{
		for (int j = 0; j < BOARD_WIDTH; j++)
		{
			sprintf(stringToHelp, "%d", arr[i][j]);
			strcat(stringToLog, stringToHelp);
			strcat(stringToLog, ";");
		}
	}
	strcat(stringToLog, "\0");
	TransferResult_t sendResult1 = ServerUtils_SendString(stringToLog, clientsSocketArr[0]);
	TransferResult_t sendResult2 = ServerUtils_SendString(stringToLog, clientsSocketArr[1]);
	if (TRNS_FAILED == sendResult1 || TRNS_FAILED == sendResult2)
	{
		return FAILURE_CODE;
	}
	else
	{
		return SUCCESS_CODE;
	}
}

int ServerUtils_TurnSwitch()
{
	char stringToLog[LENGTH_OF_SEND_STRING] = { 0 };
	strcpy(stringToLog, "TURN_SWITCH:");
	if (1 == turn)
	{
		strcat(stringToLog, usernames[0]);
	}
	else
	{
		strcat(stringToLog, usernames[1]);
	}
	TransferResult_t sendResult1 = ServerUtils_SendString(stringToLog, clientsSocketArr[0]);
	TransferResult_t sendResult2 = ServerUtils_SendString(stringToLog, clientsSocketArr[1]);

	if (sendResult1 == TRNS_FAILED || sendResult2 == TRNS_FAILED)
		return FAILURE_CODE;
	else
		return SUCCESS_CODE;
}

int ServerUtils_GameStarted()
{
	char stringToSend[LENGTH_OF_SEND_STRING] = { 0 };
	strcpy_s(stringToSend, LENGTH_OF_SEND_STRING, "GAME_STARTED");
	gameEnded = FALSE;
	TransferResult_t sendResult1 = ServerUtils_SendString(stringToSend, clientsSocketArr[0]);
	TransferResult_t sendResult2 = ServerUtils_SendString(stringToSend, clientsSocketArr[1]);

	if (TRNS_FAILED == sendResult1 || TRNS_FAILED == sendResult2)
		return FAILURE_CODE;
	else
		return SUCCESS_CODE;
}