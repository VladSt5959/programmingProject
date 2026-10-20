#include <stdio.h>
#include <stdint.h>
#include "pico/time.h"

#define SCREEN_WIDTH 320     // ширина екрана
#define SCREEN_HEIGHT 240    // висота екрана

#define CELL_SIZE 12         // одна клітинка 12×12 px

#define BOARD_WIDTH 10       // 10 клітинок по ширині
#define BOARD_HEIGHT 20      // 20 клітинок по висоті

int canmoveDown(int board[20][10], int figure[4][4], int figureX, int figureY)
{
int nextY = figureY + 1;
for (int row = 0; row < 4; row++)
    {
        for (int column = 0; column < 4; column++)
        {
            if (figure[row][column] == 1)
            {
                if (nextY + row >= 20)
                {
                    return 0;
                }
                if (board[nextY + row][figureX + column] == 1)
                {
                    return 0;
                }
            }
        }
    }
return 1; // можна рухатись вниз
} 
int canmoveLeft(int board[20][10], int figure[4][4], int figureX, int figureY)
{
    for (int row = 0; row < 4; row++)
    {
        for (int column = 0; column < 4; column++)
        {
            if (figure[row][column] == 1)
            {
                if (figureX + column - 1 < 0)           
                {
                return 0;
                }
                if (board[figureY + row][figureX + column - 1] == 1)
                {
                return 0;
                }    
            }
        }
    }
return 1;
}
int canmoveright(int board[20][10], int figure [4][4], int figureX, int figureY)
 {
    for (int row = 0; row < 4; row++)
    {
        for (int column = 0; column < 4; column++)
        {
            if (figure[row][column] == 1)
            {
                if (figureX + column + 1 >= 10)
                {
                return 0;
                }
                if (board[figureY + row][figureX + column + 1] == 1)
                {
                return 0;
                }
            }
        }
    }
    return 1;
 }
    void placeFigure(int board[20][10], int figure[4][4], int figureX, int figureY)
{
for (int row = 0; row < 4; row++)
    {
        for (int column = 0; column < 4; column++)
        {
            if (figure[row][column] == 1)
            {
                board[figureY + row][figureX + column] = 1;
            }
        }
    }
}



int rowfull(int board[20][10], int row)
{
    for(int j = 0; j < 10; j++)
    {
        if (board[row][j] == 0)
        {
            return 0;
        }
    }
    return 1;
}



// Видалення заповненого рядка
 void removeRow(int board[20][10], int row)
{
    for (int i = row; i > 0; i--)
    {
        for (int j = 0; j < 10; j++)
        {
            board[i][j] = board[i-1][j];
        }
    }
    for (int j = 0; j < 10; j++)
    {
        board[0][j] = 0;
    }
}


// Перевірка всіх рядків
void checkRows(int board[20][10])
{
    for (int i = 19; i >= 0; i--)
    {
        while (rowfull(board, i))
        {
            removeRow(board, i);
        }
    }
}




int canSpawnFigure(int board[20][10], int figure[4][4], int figureX, int figureY)
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (figure[i][j] == 1)
            {
                if (figureX + j >= 10)
                {
                    return 0;
                }

                if (figureX + j < 0)
                {
                    return 0;
                }

                if (figureY + i >= 20)
                {
                    return 0;
                }

                if (figureY + i < 0)
                {
                    return 0;
                }

                if (board[figureY + i][figureX + j] == 1)
                {
                    return 0;
                }
            }
        }
    }
    return 1;
}

int main()
{
    int board[20][10] = {0};
    int figure[4][4] =
    {
        {0, 0, 0, 0},
        {1, 1, 1, 1},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    };

    int figureX = 3;
    int figureY = 0;

    int gameRunning = 1;

    uint64_t lastFallTime = time_us_64();
    uint64_t fallInterval = 500000;

    while (gameRunning == 1)
    {
        uint64_t currentTime = time_us_64();

        if (currentTime - lastFallTime >= fallInterval)
        {
            if (canmoveDown(board, figure, figureX, figureY))
            {
                figureY++;
            }
            else
            {
                placeFigure(board, figure, figureX, figureY);
                checkRows(board);

                figureX = 3;
                figureY = 0;
                if (canSpawnFigure(board, figure, figureX, figureY) == 0)
                {
                    gameRunning = 0;
                }
            }
            lastFallTime = currentTime;
        }
    }
    return 0;
}