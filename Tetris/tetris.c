#include <stdio.h>
#define SCREEN_WIDTH 320     // ширина екрана
#define SCREEN_HEIGHT 240    // висота екрана

#define CELL_SIZE 12         // одна клітинка 12×12 px

#define BOARD_WIDTH 10       // 10 клітинок по ширині
#define BOARD_HEIGHT 20      // 20 клітинок по висоті

int canMoveDown(int board[20][10], int figure[4][4], int figureX, int figureY)
{
    for (int row = 0; row < 4; row++)
    {
        for (int column =0 ; column < 4; column++)
        {
            if (figure[row][column] == 1)
            {
            if (figureY + row + 1 >=20)
                {
                    return 0; // не можна рухатись вниз, бо досягли нижньої межі
                }
                      if (board[figureY + row + 1][figureX + column] == 1)
                {
                    return 0; // не можна рухатись вниз, бо є фігура під поточною
                }
            }
        }
    }
    return 1; // можна рухатись вниз
}
int main()
{
    int board[20][10] = {0};           // саме поле Tetris
    int figure [4] [4] = 
    {
     {0, 0, 0, 0},
     {1, 1, 1, 1},
     {0, 0, 0, 0},
     {0, 0, 0, 0}
    };
    int figureX = 3;  // падає з середини поля
    int figureY = 0;  // падає зверху поля
    if (canMoveDown(board, figure, figureX, figureY))
    {
        figureY++; // рухаємо фігуру вниз
    }

}