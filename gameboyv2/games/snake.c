#include "snake.h"
#include "colors.h"
#include "gfx.h"
#include "pico/stdlib.h"
#include "libs/ownLibs/btnReader.h"
#include <stdlib.h>

#define CELL 16
#define FIELD_X 8
#define FIELD_Y 32
#define FIELD_W 304
#define FIELD_H 192
#define COLS (FIELD_W / CELL)
#define ROWS (FIELD_H / CELL)
#define MAX_LENGTH (COLS * ROWS)

static int snakeLength, dirX, dirY;
static int snakeX[MAX_LENGTH], snakeY[MAX_LENGTH];
static int appleX, appleY, score;
static bool gameOver;
static uint32_t nextStepAt;


static void drawApple(void) {
    draw_image_scaled(FIELD_X + appleX * CELL + 4, FIELD_Y + appleY * CELL + 4,
                      8, 8, 1, appleSprite);
}

static void spawnApple(void) {
    if (snakeLength >= MAX_LENGTH) {
        gameOver = true;
        return;
    }
    bool occupied;
    do {
        occupied = false;
        appleX = rand() % COLS;
        appleY = rand() % ROWS;
        for (int i = 0; i < snakeLength; i++) {
            if (snakeX[i] == appleX && snakeY[i] == appleY) {
                occupied = true;
                break;
            }
        }
    } while (occupied);
}

void snakeInit(void) {
    GFX_fillScreen(0x0841);
    snakeLength = 4;
    dirX = 1;
    dirY = 0;
    score = 0;
    gameOver = false;
    snakeX[0] = 8; snakeY[0] = 5;
    snakeX[1] = 7; snakeY[1] = 5;
    snakeX[2] = 6; snakeY[2] = 5;
    snakeX[3] = 5; snakeY[3] = 5;

    // White frame with a red inner edge marks the playable screen boundary.
    GFX_drawRect(0, 0, 320, 240, WHITE);
    GFX_drawRect(2, 2, 316, 236, RED);
    GFX_setTextColor(WHITE);
    GFX_setTextBack(0x0841);
    GFX_setTextSize(1);
    GFX_setCursor(10, 10);
    GFX_printf("SNAKE   SCORE 0");

    spawnApple();
    for (int i = 1; i < snakeLength; i++)
        GFX_fillRect(FIELD_X + snakeX[i] * CELL + 2, FIELD_Y + snakeY[i] * CELL + 2,
                     CELL - 4, CELL - 4, 0x05A0);
    draw_image_scaled(FIELD_X + snakeX[0] * CELL + 4, FIELD_Y + snakeY[0] * CELL + 4,
                      8, 8, 1, snakeHeadSprite);
    drawApple();
    GFX_flush();
    nextStepAt = to_ms_since_boot(get_absolute_time()) + 130;
}

int snakeRead(void) {
    if (btnPressed(BTN_B)) {
        return 1;
    }

    if (gameOver) {
        if (btnPressed(BTN_A)) snakeInit();
        return 0;
    }

    if (btnPressed(BTN_UP) && dirY == 0) { dirX = 0; dirY = -1; }
    else if (btnPressed(BTN_DOWN) && dirY == 0) { dirX = 0; dirY = 1; }
    else if (btnPressed(BTN_LEFT) && dirX == 0) { dirX = -1; dirY = 0; }
    else if (btnPressed(BTN_RIGHT) && dirX == 0) { dirX = 1; dirY = 0; }
    return 0;
}

void snakeUpdate(void) {
    uint32_t now = to_ms_since_boot(get_absolute_time());
    if (gameOver || (int32_t)(now - nextStepAt) < 0) return;
    nextStepAt = now + 130;

    int nx = snakeX[0] + dirX;
    int ny = snakeY[0] + dirY;
    bool eating = nx == appleX && ny == appleY;

    if (nx < 0 || nx >= COLS || ny < 0 || ny >= ROWS) gameOver = true;
    for (int i = 0; i < snakeLength && !gameOver; i++) {
        //check last point of tail 
        if (snakeX[i] == nx && snakeY[i] == ny && (eating || i != snakeLength - 1))
            gameOver = true;
    }

    if (gameOver) {
        GFX_fillRect(55, 105, 210, 35, 0x0841);
        GFX_setTextColor(RED);
        GFX_setTextSize(2);
        GFX_setCursor(80, 105);
        GFX_printf("GAME OVER");
        GFX_setTextColor(WHITE);
        GFX_setTextSize(1);
        GFX_setCursor(76, 132);
        GFX_printf("A: restart  B: menu");
        GFX_flush();
        return;
    }

    int oldTailX = snakeX[snakeLength - 1];
    int oldTailY = snakeY[snakeLength - 1];
    if (eating && snakeLength < MAX_LENGTH) snakeLength++;
    for (int i = snakeLength - 1; i > 0; i--) {
        snakeX[i] = snakeX[i - 1];
        snakeY[i] = snakeY[i - 1];
    }
    snakeX[0] = nx;
    snakeY[0] = ny;

    if (eating) {
        score += 10;
        spawnApple();
        GFX_fillRect(8, 8, 145, 17, 0x0841);
    GFX_drawRect(0, 0, 320, 240, WHITE);
    GFX_drawRect(2, 2, 316, 236, RED);
        GFX_setTextColor(WHITE);
        GFX_setTextSize(1);
        GFX_setCursor(10, 10);
        GFX_printf("SNAKE   SCORE %d", score);
        drawApple();
    } else {
        GFX_fillRect(FIELD_X + oldTailX * CELL + 1, FIELD_Y + oldTailY * CELL + 1,
                     CELL - 2, CELL - 2, 0x0841);
    }

    for (int i = 1; i < snakeLength; i++)
        GFX_fillRect(FIELD_X + snakeX[i] * CELL + 2, FIELD_Y + snakeY[i] * CELL + 2,
                     CELL - 4, CELL - 4, 0x05A0);
    draw_image_scaled(FIELD_X + snakeX[0] * CELL + 4, FIELD_Y + snakeY[0] * CELL + 4,
                      8, 8, 1, snakeHeadSprite);
    GFX_flush();
}
