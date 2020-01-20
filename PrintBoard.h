#ifndef PRINT_BOARD_H
#define PRINT_BOARD_H

#include <windows.h>

#define RED_PLAYER 1 
#define YELLOW_PLAYER 2
#define BOARD_EMPTY_SLOT 3

#define BOARD_HEIGHT 6
#define BOARD_WIDTH  7

void PrintBoard(int board[][BOARD_WIDTH], HANDLE consoleHandle);
#endif // !PRINT_BOARD_H
