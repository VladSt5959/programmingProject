#include <stdio.h>
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

    while (canmoveDown(board, figure, figureX, figureY))
    {
        figureY++;
    }
    placeFigure(board, figure, figureX, figureY);
    
    if (canmoveLeft(board, figure, figureX, figureY))
    {
        figureX--;
    }
    if (canmoveright(board, figure, figureX, figureY))
        figureX++;
    return 0;
}