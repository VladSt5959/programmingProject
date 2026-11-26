#include "tetris.h"
#include "colors.h"
#include "gfx.h"
#include "pico/stdlib.h"
#include "libs/ownLibs/btnReader.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>


#define CELL_SIZE 20

#define BOARD_WIDTH 10
#define BOARD_HEIGHT 12

#define BOARD_START_X 8
#define BOARD_START_Y 0
#define POINTS_PER_ROW   100

static uint32_t score = 0;
static uint32_t clearedLines = 0;

static int board[BOARD_HEIGHT][BOARD_WIDTH];
static int figure[4][4];
static int figureX, figureY, figureType;
static int gameRunning, needRedraw;
static bool previousRotate;
static uint64_t lastFallTime;
static const uint64_t fallInterval = 500000;
static uint64_t lastMoveTime;
static uint16_t previousBoard[BOARD_HEIGHT][BOARD_WIDTH];

static int figures[7][4][4] =
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

static const uint16_t figureColors[7] =
{
    0xFFE0,
    0x07FF,
    0xF81F,
    0x07E0,
    0xF800,
    0x001F,
    0xFD20
};

int canmoveDown(int board[BOARD_HEIGHT][BOARD_WIDTH], int figure[4][4], int figureX, int figureY)
{
int nextY = figureY + 1;
for (int row = 0; row < 4; row++)
    {
        for (int column = 0; column < 4; column++)
        {
            if (figure[row][column] == 1)
            {
                if (nextY + row >= BOARD_HEIGHT)
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
return 1;
}
int canmoveLeft(int board[BOARD_HEIGHT][BOARD_WIDTH], int figure[4][4], int figureX, int figureY)
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
int canmoveright(int board[BOARD_HEIGHT][BOARD_WIDTH], int figure [4][4], int figureX, int figureY)
 {
    for (int row = 0; row < 4; row++)
    {
        for (int column = 0; column < 4; column++)
        {
            if (figure[row][column] == 1)
            {
                if (figureX + column + 1 >= BOARD_WIDTH)
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
    void placeFigure(int board[BOARD_HEIGHT][BOARD_WIDTH], int figure[4][4], int figureX, int figureY, uint16_t color)
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

int rowfull(int board[BOARD_HEIGHT][BOARD_WIDTH], int row)
{
    for(int j = 0; j < BOARD_WIDTH; j++)
    {
        if (board[row][j] == 0)
        {
            return 0;
        }
    }
    return 1;
}

 void removeRow(int board[BOARD_HEIGHT][BOARD_WIDTH], int row)
{
    for (int i = row; i > 0; i--)
    {
        for (int j = 0; j < BOARD_WIDTH; j++)
        {
            board[i][j] = board[i-1][j];
        }
    }
    for (int j = 0; j < BOARD_WIDTH; j++)
    {
        board[0][j] = 0;
    }
}

void checkRows(int board[BOARD_HEIGHT][BOARD_WIDTH])
{
    for (int i = BOARD_HEIGHT - 1; i >= 0; i--)
    {
        while (rowfull(board, i))
        {
            removeRow(board, i);

            score += POINTS_PER_ROW;
            clearedLines++;

        }
    }
}

int canSpawnFigure(int board[BOARD_HEIGHT][BOARD_WIDTH], int figure[4][4], int figureX, int figureY)
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (figure[i][j] == 1)
            {
                if (figureX + j >= BOARD_WIDTH)
                {
                    return 0;
                }

                if (figureX + j < 0)
                {
                    return 0;
                }

                if (figureY + i >= BOARD_HEIGHT)
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

void rotateFigure(int board[BOARD_HEIGHT][BOARD_WIDTH], int figure[4][4], int figureX, int figureY)
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

static uint16_t blockShade(uint16_t color, int light)
{
    int r = (color >> 11) & 31;
    int g = (color >> 5) & 63;
    int b = color & 31;

    if (light)
    {
        r += (31 - r) / 2;
        g += (63 - g) / 2;
        b += (31 - b) / 2;
    }
    else
    {
        r /= 3;
        g /= 3;
        b /= 3;
    }

    return (uint16_t)((r << 11) | (g << 5) | b);
}

static void drawBlock(int x, int y, uint16_t color)
{
    uint16_t lightColor = blockShade(color, 1);
    uint16_t darkColor = blockShade(color, 0);

    GFX_fillRect(x + 1, y + 1, CELL_SIZE - 2, CELL_SIZE - 2, color);
    GFX_fillRect(x + 1, y + 1, CELL_SIZE - 2, 2, lightColor);
    GFX_fillRect(x + 1, y + 3, 2, CELL_SIZE - 5, lightColor);
    GFX_fillRect(x + CELL_SIZE - 3, y + 3, 2, CELL_SIZE - 5, darkColor);
    GFX_fillRect(x + 1, y + CELL_SIZE - 3, CELL_SIZE - 2, 2, darkColor);
}

static void drawStats(void)
{
    GFX_fillRect(222, 97, 86, 18, COLOR_PANEL);
    GFX_setTextBack(COLOR_PANEL);
    GFX_setTextColor(COLOR_ACCENT);

    GFX_setTextSize(score > 999999u ? 1 : 2);
    GFX_setCursor(227, 98);
    GFX_printf("%lu", (unsigned long)score);

    GFX_fillRect(222, 157, 86, 18, COLOR_PANEL);
    GFX_setTextSize(clearedLines > 999999u ? 1 : 2);
    GFX_setCursor(227, 158);
    GFX_printf("%lu", (unsigned long)clearedLines);
}

static void drawGameOver(void)
{
    GFX_setTextBack(COLOR_PANEL);
    GFX_setTextColor(COLOR_GAME_OVER);
    GFX_setTextSize(1);
    GFX_setCursor(227, 193);
    GFX_printf("GAME OVER");
    GFX_setTextColor(COLOR_TEXT);
    GFX_setCursor(227, 207);
    GFX_printf("A: restart");
    GFX_setCursor(227, 218);
    GFX_printf("B: menu");
}

static void drawInterface(void)
{
    GFX_fillRect(BOARD_START_X - 2, BOARD_START_Y, 2,
                 BOARD_HEIGHT * CELL_SIZE, COLOR_BORDER);
    GFX_fillRect(BOARD_START_X + BOARD_WIDTH * CELL_SIZE, BOARD_START_Y,
                 2, BOARD_HEIGHT * CELL_SIZE, COLOR_BORDER);

    GFX_fillRect(217, 8, 95, 224, COLOR_BORDER);
    GFX_fillRect(219, 10, 91, 220, COLOR_PANEL);
    GFX_setTextBack(COLOR_PANEL);
    GFX_setTextColor(COLOR_ACCENT);
    GFX_setTextSize(2);
    GFX_setCursor(228, 24);
    GFX_printf("TETRIS");
    GFX_fillRect(222, 52, 85, 1, COLOR_BORDER);
    GFX_setTextColor(COLOR_TEXT);
    GFX_setTextSize(2);
    GFX_setCursor(227, 70);
    GFX_printf("SCORE");
    GFX_setCursor(227, 130);
    GFX_printf("LINES");
    GFX_fillRect(222, 183, 85, 1, COLOR_BORDER);
    drawStats();
}

void drawTetris(int board[BOARD_HEIGHT][BOARD_WIDTH], int figure[4][4],
                int figureX, int figureY, uint16_t currentColor)
{
    uint16_t currentBoard[BOARD_HEIGHT][BOARD_WIDTH];

    for (int i = 0; i < BOARD_HEIGHT; i++)
    {
        for (int j = 0; j < BOARD_WIDTH; j++)
        {
            currentBoard[i][j] = (uint16_t)board[i][j];
        }
    }

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
                    currentBoard[y][x] = currentColor;
                }
            }
        }
    }

    for (int i = 0; i < BOARD_HEIGHT; i++)
    {
        for (int j = 0; j < BOARD_WIDTH; j++)
        {
            if (currentBoard[i][j] != previousBoard[i][j])
            {
                int screenX = BOARD_START_X + j * CELL_SIZE;
                int screenY = BOARD_START_Y + i * CELL_SIZE;

                if (currentBoard[i][j] == 0)
                {
                    GFX_fillRect(screenX, screenY, CELL_SIZE, CELL_SIZE,
                                 COLOR_BOARD);
                }
                else
                {
                    drawBlock(screenX, screenY, currentBoard[i][j]);
                }

                previousBoard[i][j] = currentBoard[i][j];
            }
        }
    }
}

void tetrisInit(void)
{
    memset(board, 0, sizeof(board));
    memset(figure, 0, sizeof(figure));
    memset(previousBoard, 0, sizeof(previousBoard));
    figureX = 3;
    figureY = 0;
    score = 0;
    clearedLines = 0;
    gameRunning = 1;
    needRedraw = 0;
    previousRotate = false;

    srand((unsigned int)time_us_64());
    figureType = genrrateFigure(figures, figure);

    GFX_fillScreen(COLOR_BACKGROUND);
    GFX_fillRect(BOARD_START_X, BOARD_START_Y,
                 BOARD_WIDTH * CELL_SIZE, BOARD_HEIGHT * CELL_SIZE,
                 COLOR_BOARD);
    drawInterface();
    drawTetris(board, figure, figureX, figureY, figureColors[figureType]);
    GFX_flush();
    lastFallTime = time_us_64();
    lastMoveTime = lastFallTime - 20000;
}

int tetrisRead(void)
{
    if (btnPressed(BTN_B) )
        return 1;

    if (!gameRunning)
    {
        if (btnPressed(BTN_A))
        {
            tetrisInit();
            previousRotate = true;
        }
        return 0;
    }

    bool left = btnPressed(BTN_LEFT);
    bool right = btnPressed(BTN_RIGHT);
    bool down = btnPressed(BTN_DOWN);
    bool up = btnPressed(BTN_UP);
    bool a = btnPressed(BTN_A);
    bool currentRotate = up || a;
    uint64_t currentTime = time_us_64();

    if ((left || right || down) && currentTime - lastMoveTime >= 20000)
    {
        if (left && canmoveLeft(board, figure, figureX, figureY) == 1)
        {
            figureX--;
            needRedraw = 1;
        }
        if (right && canmoveright(board, figure, figureX, figureY) == 1)
        {
            figureX++;
            needRedraw = 1;
        }
        if (down && canmoveDown(board, figure, figureX, figureY) == 1)
        {
            figureY++;
            needRedraw = 1;
        }
        lastMoveTime = currentTime;
    }

    if (!previousRotate && currentRotate)
    {
        rotateFigure(board, figure, figureX, figureY);
        needRedraw = 1;
    }
    previousRotate = currentRotate;
    return 0;
}

void tetrisUpdate(void)
{
    if (!gameRunning) return;
    uint64_t currentTime = time_us_64();

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
            uint32_t previousLines = clearedLines;
            checkRows(board);

            if (clearedLines != previousLines)
            {
                drawStats();
            }

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

    if (!gameRunning)
    {
        int emptyFigure[4][4] = {0};
        drawTetris(board, emptyFigure, 0, 0, 0);
        drawGameOver();
        GFX_flush();
        needRedraw = 0;
    }
    else if (needRedraw == 1)
    {
        drawTetris(board, figure, figureX, figureY, figureColors[figureType]);
        GFX_flush();
        needRedraw = 0;
    }
}
