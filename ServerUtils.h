#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN


#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <wS2tcpip.h>
#include <windows.h>
#include <stdio.h>
#pragma comment(lib, "ws2_32.lib")

#include "Loger.h"
#include "Defines.h"
typedef enum { TRNS_FAILED, TRNS_DISCONNECTED, TRNS_SUCCEEDED } TransferResult_t;

/*
Description - IsUsernameFree function checks if a requested username is free to use
Parameters  -
input       : users - array which holds usernames, username - the username request, numberOfUsers - how many users are connected
returns     : int which indicates if free or not
*/
int IsUsernameFree(char users[][LENGTH_OF_USERNAME], char *username, int numberOfUsers);

/*
Description - ServerUtils_NewUserRequest function handles the request for new user
Parameters  -
input       : clientSocket - pointer to socket of the requesting client
returns     : success_code if succeeded, failure_code else
*/
int ServerUtils_NewUserRequest(SOCKET *clientSocket);

/*
Description - ServerUtils_ReceiveString function receives a string from socket
Parameters  -
input       : ppOutputString - char** for the string, socket - socket to receive from
returns     : trns_succeeded if good, trns_failed if not, trns_disconnected if desconnected
*/
TransferResult_t ServerUtils_ReceiveString(char **ppOutputString, SOCKET socket);

/*
Description - ServerUtils_ReceiveBuffer function receives a buffer from socket and stors it into pOutputBuffer
Parameters  -
input       : pOutputBuffer - char* for the string, socket - socket to receive from
returns     : trns_succeeded if good, trns_failed if not, trns_disconnected if desconnected
*/
TransferResult_t ServerUtils_ReceiveBuffer(char *pOutputBuffer, SOCKET socket);

/*
Description - ServerUtils_SendString function sends a string to socket
Parameters  -
input       : string - string to be sent, socket - socket to send to
returns     : trns_succeeded if good, trns_failed if not, trns_disconnected if desconnected
*/
TransferResult_t ServerUtils_SendString(char *string, SOCKET socket);

/*
Description - ServerUtils_SendBuffer function sends a buffer to socket
Parameters  -
input       : buffer - buffer to be sent, socket - socket to send to
returns     : trns_succeeded if good, trns_failed if not, trns_disconnected if desconnected
*/
TransferResult_t ServerUtils_SendBuffer(const char *buffer, int bytesToSend, SOCKET socket);

/*
Description - ServerUtils_CloseConnections function closes all of the connections made in program
Parameters  -
input       : none
returns     : none(void function)
*/
void ServerUtils_CloseConnections();

/*
Description - ServerUtils_DecodeMessage function decodes the message received from client and handles it
Parameters  -
input       : inputStringFromUser - the string received from client, playerNum - which player sent the string, clientSocket - socket of the client
returns     : success_code if all good , failure_code else
*/
int ServerUtils_DecodeMessage(char *inputStringFromUser, int playerNum, SOCKET *clientSocket);

/*
Description - AreFourConnected function checks if theres a win on the board
Parameters  -
input       : playerColor - the color of the player to check if there is a win
returns     : success_code if found , failure_code if no win
*/
int AreFourConnected(int playerColor);

/*
Description - GameEnded function handles all that need to handle if theres a win and the game is ended.
Parameters  -
input       : playerNum - the number of the player who won. BOOL tie- indicated weather theres a tie
returns     : success_code if good , failure_code if no
*/
int GameEnded(int playerNum, BOOL tie);

/*
Description - ServerUtiles_InitBoard function inits the board to play
Parameters  -
input       : none
returns     : none (void function)
*/
void ServerUtiles_InitBoard();

/*
Description - ItsATie function checks if there is a tie
Parameters  -
input       : none
returns     : true for tie, false o.w
*/
BOOL ItsATie();

/*
Description - ServerUtils_BoardView function sends the board to clients
Parameters  -
input       : none
returns     : success_code if good , failure_code if no
*/
int ServerUtils_BoardView();

/*
Description - ServerUtils_TurnSwitch function turns the switch for other player
Parameters  -
input       : none
returns     : success_code if good , failure_code if no
*/
int ServerUtils_TurnSwitch();

/*
Description - ServerUtils_GameStarted function notifies clients that game has started
Parameters  -
input       : none
returns     : success_code if good , failure_code if no
*/
int ServerUtils_GameStarted();

#endif