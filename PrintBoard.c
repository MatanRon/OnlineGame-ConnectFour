#include <stdio.h>
#include "PrintBoard.h"

#define BLACK  15
#define RED    204
#define YELLOW 238

/***********************************************************
* This function prints the board, and uses O as the holes.
* The disks are presented by red or yellow backgrounds.
* Input: A 2D array representing the board and the console handle
* Output: Prints the board, no return value
************************************************************/
void PrintBoard(int board[][BOARD_WIDTH], HANDLE consoleHandle)
{

	int row, column;
	//Draw the board
	for (row = 0; row < BOARD_HEIGHT; row++)
	{
		for (column = 0; column < BOARD_WIDTH; column++)
		{
			printf("| ");
			if (board[row][column] == RED_PLAYER)
				SetConsoleTextAttribute(consoleHandle, RED);

			else if (board[row][column] == YELLOW_PLAYER)
				SetConsoleTextAttribute(consoleHandle, YELLOW);

			printf("O");

			SetConsoleTextAttribute(consoleHandle, BLACK);
			printf(" ");
		}
		printf("\n");

		//Draw dividing line between the rows
		for (column = 0; column < BOARD_WIDTH; column++)
		{
			printf("----");
		}
		printf("\n");
	}


}