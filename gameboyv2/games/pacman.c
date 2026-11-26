#include <stdint.h>
#include "pacman.h"
#include "gfx.h"
#include "pico/stdlib.h"
#include "libs/ownLibs/btnReader.h"
#include "colors.h"

#define ROWS 17
#define COLS 25
#define CELL_SIZE 12
#define BOARD_START_X 10
#define BOARD_START_Y 27

#define PAC_BACKGROUND 0x0000
#define PAC_WALL 0x0014
#define PAC_BORDER 0x34BF
#define PAC_DOT 0xFFBD
#define PAC_YELLOW 0xFFE0
#define PAC_TEXT 0xFFFF
#define GHOST_RED 0xF800
#define GHOST_PINK 0xFCD9
#define GHOST_BLUE 0x07FF
#define GHOST_ORANGE 0xFD20
#define GHOST_EYES 0xFFFF
#define GHOST_PUPILS 0x001F

#define RIGHT 0
#define LEFT 1
#define UP 2
#define DOWN 3

#define MOVE_INTERVAL 150000
#define POWER_TICKS 40
#define GHOST_POWER 0x245F
#define PLAYING 0
#define GAME_OVER 1
#define YOU_WIN 2



static int board[ROWS][COLS];

static int pacmanX = 12;
static int pacmanY = 2;
static int score = 0;
static int lives = 3;
static int direction = RIGHT;
static int mouthOpen = 0;
static int nextDirection = RIGHT;
static int gameState = PLAYING;
static int powerTimer = 0;
static int ghostEaten[4] = {0};
static uint64_t lastMoveTime = 0;
static int previousA = 0;


#define GHOST_PATH_SIZE 46
#define GHOST_MOVE_INTERVAL 2

static const int ghostStartX[4] = {1, 23, 1, 23};
static const int ghostStartY[4] = {11, 15, 1, 1};
static int ghostX[4] = {1, 23, 1, 23};
static int ghostY[4] = {11, 15, 1, 1};
static int ghostStep[4] = {0, 0, 0, 0};
static int ghostTimer = 0;

static const int ghostPathLength[4] = {28, 40, 42, 46};

static const int ghostPath[4][GHOST_PATH_SIZE] = {
    {
        RIGHT, RIGHT, RIGHT, RIGHT, UP, UP, RIGHT, RIGHT,
        RIGHT, RIGHT, RIGHT, RIGHT, UP, UP, LEFT, LEFT,
        LEFT, LEFT, LEFT, LEFT, LEFT, LEFT, LEFT, LEFT,
        DOWN, DOWN, DOWN, DOWN
    },
    {
        LEFT, LEFT, LEFT, LEFT, LEFT, LEFT, LEFT, LEFT,
        LEFT, LEFT, DOWN, DOWN, RIGHT, RIGHT, RIGHT, RIGHT,
        DOWN, DOWN, LEFT, LEFT, DOWN, DOWN, RIGHT, RIGHT,
        LEFT, LEFT, UP, UP, RIGHT, RIGHT, UP, UP,
        RIGHT, RIGHT, RIGHT, RIGHT, RIGHT, RIGHT, UP, UP
    },
    {
        RIGHT, RIGHT, RIGHT, RIGHT, RIGHT, RIGHT, RIGHT, RIGHT,
        RIGHT, RIGHT, UP, UP, UP, LEFT, LEFT, LEFT,
        LEFT, LEFT, LEFT, UP, UP, LEFT, LEFT, LEFT,
        LEFT, UP, UP, RIGHT, RIGHT, RIGHT, RIGHT, DOWN,
        DOWN, DOWN, DOWN, LEFT, LEFT, LEFT, LEFT, DOWN,
        DOWN, DOWN
    },
    {
        LEFT, LEFT, UP, RIGHT, RIGHT, UP, UP, LEFT,
        LEFT, LEFT, LEFT, UP, UP, UP, UP, RIGHT,
        RIGHT, RIGHT, RIGHT, DOWN, DOWN, LEFT, LEFT, LEFT,
        LEFT, DOWN, DOWN, LEFT, LEFT, LEFT, LEFT, LEFT,
        LEFT, DOWN, DOWN, RIGHT, RIGHT, RIGHT, RIGHT, RIGHT,
        RIGHT, DOWN, RIGHT, RIGHT, RIGHT, RIGHT
    },
};

static uint16_t ghostColors[4] = {
    GHOST_RED, GHOST_PINK, GHOST_BLUE, GHOST_ORANGE
};

static int checkCollision(int x, int y)
{
    if (x < 0 || x >= COLS || y < 0 || y >= ROWS)
        return 1;

    if (board[y][x] == 1)
        return 1;

    return 0;
}

static void moveGhost(int i)
{
    int x = ghostX[i];
    int y = ghostY[i];
    int dir = ghostPath[i][ghostStep[i]];

    if (dir == RIGHT)
        x++;
    else if (dir == LEFT)
        x--;
    else if (dir == UP)
        y++;
    else if (dir == DOWN)
        y--;

    if (checkCollision(x, y) == 0)
    {
        ghostX[i] = x;
        ghostY[i] = y;
        ghostStep[i]++;

        if (ghostStep[i] == ghostPathLength[i])
            ghostStep[i] = 0;
    }
}

static void moveGhosts(void)
{
    ghostTimer++;

    if (ghostTimer == GHOST_MOVE_INTERVAL)
    {
        for (int i = 0; i < 4; i++)
            moveGhost(i);

        ghostTimer = 0;
    }
}

static void moveLeft(void)
{
    if (checkCollision(pacmanX - 1, pacmanY) == 0)
        pacmanX--;
}

static void moveRight(void)
{
    if (checkCollision(pacmanX + 1, pacmanY) == 0)
        pacmanX++;
}

static void moveUp(void)
{
    if (checkCollision(pacmanX, pacmanY + 1) == 0)
        pacmanY++;
}

static void moveDown(void)
{
    if (checkCollision(pacmanX, pacmanY - 1) == 0)
        pacmanY--;
}

static void eatDot(void)
{
    if (board[pacmanY][pacmanX] == 2)
    {
        board[pacmanY][pacmanX] = 0;
        score += 10;
    }
    else if (board[pacmanY][pacmanX] == 3)
    {
        board[pacmanY][pacmanX] = 0;
        score += 50;
        powerTimer = POWER_TICKS;
    }
}

static void openMouth(void)
{
    for (int j = 7; j < 10; j++)
    {
        pacmanSprite[3][j] = 0;
        pacmanSprite[6][j] = 0;
    }

    for (int j = 5; j < 10; j++)
    {
        pacmanSprite[4][j] = 0;
        pacmanSprite[5][j] = 0;
    }
}

static void closeMouth(void)
{
    for (int j = 7; j < 10; j++)
    {
        pacmanSprite[3][j] = 1;
        pacmanSprite[6][j] = 1;
    }

    for (int j = 5; j < 10; j++)
    {
        pacmanSprite[4][j] = 1;
        pacmanSprite[5][j] = 1;
    }
}

static void animateMouth(void)
{
    if (mouthOpen == 0)
    {
        openMouth();
        mouthOpen = 1;
    }
    else
    {
        closeMouth();
        mouthOpen = 0;
    }
}

static int canMove(int dir)
{
    if (dir == RIGHT)
        return checkCollision(pacmanX + 1, pacmanY) == 0;
    if (dir == LEFT)
        return checkCollision(pacmanX - 1, pacmanY) == 0;
    if (dir == UP)
        return checkCollision(pacmanX, pacmanY + 1) == 0;
    return checkCollision(pacmanX, pacmanY - 1) == 0;
}

static void movePacman(void)
{
    if (canMove(nextDirection))
        direction = nextDirection;

    int oldX = pacmanX;
    int oldY = pacmanY;

    if (direction == RIGHT)
        moveRight();
    else if (direction == LEFT)
        moveLeft();
    else if (direction == UP)
        moveUp();
    else if (direction == DOWN)
        moveDown();

    if (oldX != pacmanX || oldY != pacmanY)
        animateMouth();

    eatDot();
}

static int checkWin(void)
{
    for (int y = 0; y < ROWS; y++)
    {
        for (int x = 0; x < COLS; x++)
        {
            if (board[y][x] == 2 || board[y][x] == 3)
                return 0;
        }
    }

    return 1;
}

static void resetPositions(void)
{
    pacmanX = 12;
    pacmanY = 2;
    direction = RIGHT;
    nextDirection = RIGHT;
    closeMouth();
    mouthOpen = 0;
    powerTimer = 0;
    ghostTimer = 0;

    for (int i = 0; i < 4; i++)
    {
        ghostX[i] = ghostStartX[i];
        ghostY[i] = ghostStartY[i];
        ghostStep[i] = 0;
        ghostEaten[i] = 0;
    }
}

static void loseLife(void)
{
    lives--;
    if (lives == 0)
        gameState = GAME_OVER;
    else
        resetPositions();
}

static int checkGhostCollision(void)
{
    for (int i = 0; i < 4; i++)
    {
        if (ghostEaten[i] == 0 &&
            pacmanX == ghostX[i] && pacmanY == ghostY[i])
        {
            if (powerTimer > 0)
            {
                ghostEaten[i] = 1;
                score += 200;
            }
            else
            {
                loseLife();
                return 1;
            }
        }
    }
    return 0;
}

static void updatePower(void)
{
    if (powerTimer > 0)
        powerTimer--;

    if (powerTimer == 0)
    {
        for (int i = 0; i < 4; i++)
        {
            if (pacmanX != ghostX[i] || pacmanY != ghostY[i])
                ghostEaten[i] = 0;
        }
    }
}

static void drawCell(int x, int y)
{
    int screenX = BOARD_START_X + x * CELL_SIZE;
    int screenY = BOARD_START_Y + (ROWS - 1 - y) * CELL_SIZE;

    if (board[y][x] == 1)
    {
        GFX_fillRect(screenX, screenY, CELL_SIZE, CELL_SIZE, PAC_WALL);

        if (x == 0 || board[y][x - 1] != 1)
            GFX_fillRect(screenX, screenY, 1, CELL_SIZE, PAC_BORDER);
        if (x == COLS - 1 || board[y][x + 1] != 1)
            GFX_fillRect(screenX + CELL_SIZE - 1, screenY, 1, CELL_SIZE, PAC_BORDER);
        if (y == ROWS - 1 || board[y + 1][x] != 1)
            GFX_fillRect(screenX, screenY, CELL_SIZE, 1, PAC_BORDER);
        if (y == 0 || board[y - 1][x] != 1)
            GFX_fillRect(screenX, screenY + CELL_SIZE - 1, CELL_SIZE, 1, PAC_BORDER);
    }
    else
    {
        GFX_fillRect(screenX, screenY, CELL_SIZE, CELL_SIZE, PAC_BACKGROUND);
        if (board[y][x] == 2)
        {
            GFX_fillRect(screenX + 5, screenY + 4, 2, 4, PAC_DOT);
            GFX_fillRect(screenX + 4, screenY + 5, 4, 2, PAC_DOT);
        }
        else if (board[y][x] == 3)
        {
            GFX_fillRect(screenX + 4, screenY + 2, 4, 8, PAC_YELLOW);
            GFX_fillRect(screenX + 2, screenY + 4, 8, 4, PAC_YELLOW);
            GFX_fillRect(screenX + 3, screenY + 3, 6, 6, PAC_YELLOW);
        }
    }
}

static void drawBoard(void)
{
    for (int y = 0; y < ROWS; y++)
        for (int x = 0; x < COLS; x++)
            drawCell(x, y);
}

static void drawPacman(void)
{
    int screenX = BOARD_START_X + pacmanX * CELL_SIZE;
    int screenY = BOARD_START_Y + (ROWS - 1 - pacmanY) * CELL_SIZE;

    for (int y = 0; y < 10; y++)
    {
        for (int x = 0; x < 10; x++)
        {
            int pixelX = x;
            int pixelY = y;

            if (direction == LEFT)
                pixelX = 9 - x;
            else if (direction == UP)
            {
                pixelX = y;
                pixelY = 9 - x;
            }
            else if (direction == DOWN)
            {
                pixelX = 9 - y;
                pixelY = x;
            }

            if (pacmanSprite[y][x] == 1)
                GFX_fillRect(screenX + pixelX + 1, screenY + pixelY + 1,
                             1, 1, PAC_YELLOW);
        }
    }
}

static void drawGhost(int x, int y, uint16_t color)
{
    int sprite[10][10] = {
        {0, 0, 0, 1, 1, 1, 1, 0, 0, 0},
        {0, 0, 1, 1, 1, 1, 1, 1, 0, 0},
        {0, 1, 1, 1, 1, 1, 1, 1, 1, 0},
        {1, 1, 2, 2, 1, 1, 2, 2, 1, 1},
        {1, 1, 2, 3, 1, 1, 2, 3, 1, 1},
        {1, 1, 2, 2, 1, 1, 2, 2, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 0, 1, 1, 0, 1, 1, 1},
        {1, 1, 0, 0, 1, 1, 0, 0, 1, 1}
    };

    int screenX = BOARD_START_X + x * CELL_SIZE;
    int screenY = BOARD_START_Y + (ROWS - 1 - y) * CELL_SIZE;

    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            if (sprite[i][j] == 1)
                GFX_fillRect(screenX + j + 1, screenY + i + 1, 1, 1, color);
            else if (sprite[i][j] == 2)
                GFX_fillRect(screenX + j + 1, screenY + i + 1, 1, 1, GHOST_EYES);
            else if (sprite[i][j] == 3)
                GFX_fillRect(screenX + j + 1, screenY + i + 1, 1, 1, GHOST_PUPILS);
        }
    }
}

static void drawGhosts(void)
{
    for (int i = 0; i < 4; i++)
    {
        if (ghostEaten[i] == 0)
        {
            if (powerTimer > 0)
                drawGhost(ghostX[i], ghostY[i], GHOST_POWER);
            else
                drawGhost(ghostX[i], ghostY[i], ghostColors[i]);
        }
    }
}

static void drawScore(void)
{
    GFX_fillRect(0, 0, 320, 26, PAC_BACKGROUND);
    GFX_setTextSize(2);
    GFX_setTextColor(PAC_TEXT);
    GFX_setTextBack(PAC_BACKGROUND);

    GFX_setCursor(10, 4);
    GFX_printf("SCORE: %d", score);

    GFX_setCursor(214, 4);
    GFX_printf("LIVES: %d", lives);

    if (powerTimer > 0)
    {
        GFX_setTextSize(1);
        GFX_setTextColor(PAC_YELLOW);
        GFX_setCursor(155, 8);
        GFX_printf("POWER %d", (powerTimer * MOVE_INTERVAL + 999999) / 1000000);
    }
}

static void drawGame(void)
{
    GFX_fillScreen(PAC_BACKGROUND);
    drawBoard();
    drawPacman();
    drawGhosts();
    drawScore();
    GFX_flush();
}

static void drawResult(void)
{
    GFX_fillScreen(PAC_BACKGROUND);
    GFX_setTextBack(PAC_BACKGROUND);
    GFX_setTextSize(3);
    if (gameState == GAME_OVER)
    {
        GFX_setTextColor(GHOST_RED);
        GFX_setCursor(80, 65);
        GFX_printf("GAME OVER");
    }
    else
    {
        GFX_setTextColor(PAC_YELLOW);
        GFX_setCursor(98, 65);
        GFX_printf("YOU WIN");
    }
    GFX_setTextSize(2);
    GFX_setTextColor(PAC_TEXT);
    GFX_setCursor(88, 110);
    GFX_printf("SCORE: %d", score);
    GFX_setTextSize(1);
    GFX_setCursor(112, 161);
    GFX_printf("A - RESTART");
    GFX_setCursor(112, 179);
    GFX_printf("B - MENU");
    GFX_flush();
}

void pacmanInit(void)
{
    for (int y = 0; y < ROWS; y++)
        for (int x = 0; x < COLS; x++)
            board[y][x] = initialBoard[y][x];

    board[2][1] = 3;
    board[2][23] = 3;
    board[14][1] = 3;
    board[14][23] = 3;

    score = 0;
    lives = 3;
    gameState = PLAYING;
    resetPositions();
    previousA = btnPressed(BTN_A);
    lastMoveTime = time_us_64();
    drawGame();
}

int pacmanRead(void)
{
    if (btnPressed(BTN_B))
        return 1;

    int a = btnPressed(BTN_A);
    if (gameState != PLAYING)
    {
        if (a && !previousA)
            pacmanInit();
    }
    else
    {
        if (btnPressed(BTN_UP))
            nextDirection = UP;
        else if (btnPressed(BTN_DOWN))
            nextDirection = DOWN;
        else if (btnPressed(BTN_LEFT))
            nextDirection = LEFT;
        else if (btnPressed(BTN_RIGHT))
            nextDirection = RIGHT;
    }
    previousA = a;
    return 0;
}

static void updateGame(void)
{
    movePacman();

    if (checkGhostCollision())
    {
        if (gameState == PLAYING && checkWin())
            gameState = YOU_WIN;
        return;
    }

    moveGhosts();

    if (checkGhostCollision() && gameState == GAME_OVER)
        return;

    if (checkWin())
        gameState = YOU_WIN;
}

void pacmanUpdate(void)
{
    if (gameState != PLAYING)
        return;

    uint64_t currentTime = time_us_64();
    if (currentTime - lastMoveTime < MOVE_INTERVAL)
        return;
    lastMoveTime = currentTime;

    int oldX = pacmanX;
    int oldY = pacmanY;
    int oldGhostX[4];
    int oldGhostY[4];
    int oldScore = score;
    int oldLives = lives;
    int oldPower = powerTimer;

    for (int i = 0; i < 4; i++)
    {
        oldGhostX[i] = ghostX[i];
        oldGhostY[i] = ghostY[i];
    }

    updatePower();
    updateGame();

    if (gameState != PLAYING)
    {
        drawResult();
        return;
    }

    drawCell(oldX, oldY);
    drawCell(pacmanX, pacmanY);
    for (int i = 0; i < 4; i++)
    {
        drawCell(oldGhostX[i], oldGhostY[i]);
        drawCell(ghostX[i], ghostY[i]);
    }

    drawGhosts();
    drawPacman();

    if (oldScore != score || oldLives != lives || (oldPower * MOVE_INTERVAL + 999999) / 1000000 !=
        (powerTimer * MOVE_INTERVAL + 999999) / 1000000)
        drawScore();

    GFX_flush();
}
