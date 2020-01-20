#ifndef LOGER_H
#define LOGER_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <wS2tcpip.h>
#include <windows.h>
#include <stdio.h>

#pragma comment(lib, "ws2_32.lib")


FILE *pLogerFile;

#define _WINSOCK_DEPRECATED_NO_WARNINGS
/*
Description - this function prints a line to log file and screen
Parameters  - 
input       : a char pointer to the line
onlyToLog   : BOOL which indicates writing to screen or not. if 1 writes to log only.
returns     : none (void function)
*/
void Loger_WriteToLogAndPrint(char *input, BOOL onlyToLog);

#endif