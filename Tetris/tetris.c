#include <stdlib.h>
#include <stdint.h>
#include "pico/time.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "ili9341.h"

// Тимчасові номери GPIO
#define BUTTON_LEFT 14
#define BUTTON_RIGHT 15
#define BUTTON_DOWN 16
#define BUTTON_ROTATE 17

#define SCREEN_WIDTH 320     // ширина екрана
#define SCREEN_HEIGHT 240    // висота екрана

#define CELL_SIZE 12         // одна клітинка 12×12 px

#define BOARD_WIDTH 10       // 10 клітинок по ширині
#define BOARD_HEIGHT 20      // 20 клітинок по висоті

#define BOARD_START_X 20
#define BOARD_START_Y 0

#define COLOR_BACKGROUND 0x0884
#define COLOR_BOARD      0x1947
#define COLOR_BORDER     0x9517
#define COLOR_GRID       0x42AD

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
                if (board[nextY + row][figureX + column] != 0)
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
                if (board[figureY + row][figureX + column - 1] != 0)
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
                if (board[figureY + row][figureX + column + 1] != 0)
                {
                return 0;
                }
            }
        }
    }
    return 1;
 }
    void placeFigure(int board[20][10], int figure[4][4], int figureX, int figureY, uint16_t color)
{
for (int row = 0; row < 4; row++)
    {
        for (int column = 0; column < 4; column++)
        {
            if (figure[row][column] == 1)
            {
                board[figureY + row][figureX + column] = color;
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

                if (board[figureY + i][figureX + j] != 0)
                {
                    return 0;
                }
            }
        }
    }
    return 1;
}



void rotateFigure(int board[20][10], int figure[4][4], int figureX, int figureY)
{
    int rotated[4][4] = {0};
    for (int row = 0; row < 4; row++)
    {
        for (int column = 0; column < 4; column++)
        {
            rotated[column][3 - row] = figure[row][column];
        }
    }
    if (canSpawnFigure(board, rotated, figureX, figureY) == 1)
    {
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                figure[i][j] = rotated[i][j];
            }
        }
    }
}

int genrrateFigure(int figures[7][4][4], int figure[4][4])
{
    int randomFigure = rand() % 7;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            figure[i][j] = figures[randomFigure][i][j];
        }
    }
    return randomFigure;
}

//Ініціалізація кнопок
void initButton(uint pin)
{
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);
    gpio_pull_up(pin);
}


// Малювання поля та кольорових фігур
void drawTetris(int board[20][10], int figure[4][4],
                int figureX, int figureY, uint16_t currentColor)
{
    LCD_fillRect(
        BOARD_START_X,
        BOARD_START_Y,
        BOARD_WIDTH * CELL_SIZE,
        BOARD_HEIGHT * CELL_SIZE,
        COLOR_BOARD
    );

    // Малюємо зафіксовані блоки
    for (int i = 0; i < BOARD_HEIGHT; i++)
    {
        for (int j = 0; j < BOARD_WIDTH; j++)
        {
            if (board[i][j] != 0)
            {
                int screenX = BOARD_START_X + j * CELL_SIZE;
                int screenY = BOARD_START_Y + i * CELL_SIZE;

                LCD_fillRect(
                    screenX + 1,
                    screenY + 1,
                    CELL_SIZE - 2,
                    CELL_SIZE - 2,
                    (uint16_t)board[i][j]
                );
            }
        }
    }

    // Малюємо поточну фігуру
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (figure[i][j] == 1)
            {
                int x = figureX + j;
                int y = figureY + i;

                if (x >= 0 && x < BOARD_WIDTH &&
                    y >= 0 && y < BOARD_HEIGHT)
                {
                    int screenX = BOARD_START_X + x * CELL_SIZE;
                    int screenY = BOARD_START_Y + y * CELL_SIZE;

                    LCD_fillRect(
                        screenX + 1,
                        screenY + 1,
                        CELL_SIZE - 2,
                        CELL_SIZE - 2,
                        currentColor
                    );
                }
            }
        }
    }
}




int main()
{
    int board[20][10] = {0};
    int figure[4][4] = {0};

    int figures[7][4][4] =
    {
        {
        {0, 0, 0, 0},
        {0, 1, 1, 0},
        {0, 1, 1, 0},
        {0, 0, 0, 0}
        },

        {
        {0, 0, 0, 0},
        {1, 1, 1, 1},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
        },

        {
        {0, 1, 0, 0},
        {1, 1, 1, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
        },

        {
        {0, 1, 1, 0},
        {1, 1, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
        },

        {
        {1, 1, 0, 0},
        {0, 1, 1, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
        },

        {
        {1, 0, 0, 0},
        {1, 1, 1, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
        },

        {
        {0, 0, 1, 0},
        {1, 1, 1, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
        }
    };

    uint16_t figureColors[7] =
    {
        0xFFE0, // O - жовтий
        0x07FF, // I - блакитний
        0x780F, // T - фіолетовий
        0x07E0, // S - зелений
        0xF800, // Z - червоний
        0x001F, // J - синій
        0xFD20  // L - помаранчевий
    };

    int figureX = 3;
    int figureY = 0;

    int gameRunning = 1;

        // Попередній стан кнопки обертання
    int previousRotate = 1;

        // Ініціалізація кнопок
    initButton(BUTTON_LEFT);
    initButton(BUTTON_RIGHT);
    initButton(BUTTON_DOWN);
    initButton(BUTTON_ROTATE);

    uint64_t lastFallTime = time_us_64();
    uint64_t fallInterval = 500000;
    srand((unsigned int)time_us_64());
    int figureType = genrrateFigure(figures, figure);

    //призначення контактів дисплея ILI9341
    LCD_setPins(20, 21, 22, 18, 19);
    LCD_setSPIperiph(spi0);
    LCD_initDisplay();
    LCD_setRotation(1);
    LCD_fillScreen(COLOR_BACKGROUND);

    drawTetris(board, figure, figureX, figureY, figureColors[figureType]);
    // Головний ігровий цикл
    while (gameRunning == 1)
    {
        int needRedraw = 0;
        uint64_t currentTime = time_us_64();

        // Рух вліво
        if (gpio_get(BUTTON_LEFT) == 0)
        {
            if (canmoveLeft(board, figure, figureX, figureY) == 1)
            {
                figureX--;
                needRedraw = 1;
            }
        }
        // Рух вправо
        if (gpio_get(BUTTON_RIGHT) == 0)
        {
            if (canmoveright(board, figure, figureX, figureY) == 1)
            {
                figureX++;
                needRedraw = 1;
            }
        }
        // Рух вниз
        if (gpio_get(BUTTON_DOWN) == 0)
        {
            if (canmoveDown(board, figure, figureX, figureY) == 1)
            {
                figureY++;
                needRedraw = 1;
            }
        }

        // Обертання фігури
        int currentRotate = gpio_get(BUTTON_ROTATE);

        if (previousRotate == 1 && currentRotate == 0)
        {
            rotateFigure(board, figure, figureX, figureY);
            needRedraw = 1;
        }

        previousRotate = currentRotate;

        // Автоматичне падіння
        if (currentTime - lastFallTime >= fallInterval)
        {
            if (canmoveDown(board, figure, figureX, figureY))
            {
                figureY++;
                needRedraw = 1;
            }
            else
            {
                placeFigure(board, figure, figureX, figureY, figureColors[figureType]);
                checkRows(board);

                figureType = genrrateFigure(figures, figure);

                figureX = 3;
                figureY = 0;
                if (canSpawnFigure(board, figure, figureX, figureY) == 0)
                {
                    gameRunning = 0;
                }
            }
            lastFallTime = currentTime;
            needRedraw = 1;
        }

        if (needRedraw == 1 && gameRunning == 1)
        {
            drawTetris(board, figure, figureX, figureY,
                       figureColors[figureType]);
        }
        sleep_ms(20);
    }
    return 0;
}