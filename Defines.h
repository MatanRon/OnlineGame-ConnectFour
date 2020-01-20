#ifndef DEFINES_H
#define DEFINES_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <windows.h>
#include <wS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")


#include <string.h>
#include <stdlib.h>





#define SUCCESS_CODE 0
#define FAILURE_CODE (-1)
#define GAME_ENDED 2

#define ARG_LOGFILE 2
#define ARG_PORT 3

#define LENGTH_OF_USERNAME 32
#define LENGTH_OF_SEND_STRING 300
#define SERVER_ADDRESS_STR "127.0.0.1"
#define NUM_OF_PLAYERS 2
#define BOARD_HEIGHT 6
#define BOARD_WIDTH 7
#define BLACK 15
#define RED 204
#define YELLOW 238
#define RED_PLAYER 1
#define YELLOW_PLAYER 2

#define MESSAGE_SEPARATOR_CHAR (';') 
#define MESSAGE_SEPARATOR_STRING ";" // Without parenthese to allow string concat
#define CHAT_MESSAGE_SEPARATOR_STRING ("; ;")

#define TERMINATED_CODE (1)

#define LONGEST_MUTEX_WAITING_TIME_MS (10000)
#define LONGEST_TURN_TIME_MS (10 * 60 * 1000)
#define BASE (10)
#define USER_NAME_MAX_LENGTH (31)
#define MESSAGE_MAX_LENGTH (300)

//GLOBAL VARIABLES---------------------------------------------
char usernames[NUM_OF_PLAYERS][LENGTH_OF_USERNAME];
int arr[BOARD_HEIGHT][BOARD_WIDTH];
int numOfConnections;
int turn;
int serverPort;
char pLogFilePath[1000];

extern BOOL playerDisconnected;
extern volatile BOOL gameEnded;
extern SOCKET clientsSocketArr[NUM_OF_PLAYERS];
extern HANDLE clientsThreads[NUM_OF_PLAYERS];
extern HANDLE mutexOrderingResponses;
extern HANDLE mutexWriteToLog;

#define STRINGS_ARE_EQUAL(str1,str2) ( strcmp((str1), (str2)) == 0 )





#endif // !DEFINES_H
