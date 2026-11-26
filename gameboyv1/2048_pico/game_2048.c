#include "game2048.h"

#include "pico/stdlib.h"
#include "libs/ownLibs/btnReader.h"
#include "libs/gfx/gfx.h"
#include "colors.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define G2048_SCREEN_W      320
#define G2048_SCREEN_H      240
#define G2048_HUD_H         20

#define G2048_GRID_SIZE     4
#define G2048_TILE_SIZE     44
#define G2048_TILE_GAP      6
#define G2048_BOARD_SIZE    (G2048_GRID_SIZE * G2048_TILE_SIZE + (G2048_GRID_SIZE + 1) * G2048_TILE_GAP)
#define G2048_BOARD_X       ((G2048_SCREEN_W - G2048_BOARD_SIZE) / 2)
#define G2048_BOARD_Y       (G2048_HUD_H + (G2048_SCREEN_H - G2048_HUD_H - G2048_BOARD_SIZE) / 2)

#define G2048_FRAME_DELAY   30

#define G2048_COL_BG        0xFFDD // #FAF8EF 
#define G2048_COL_BOARD     0xBD74 // #BBADA0 
#define G2048_COL_EMPTY     0xCE16 // #CDC1B4 
#define G2048_COL_TEXT_DARK 0x736C // #776E65 
#define G2048_COL_TEXT_LITE 0xFFBE // #F9F6F2 

#define G2048_COL_2         0xEF3B // #EEE4DA 
#define G2048_COL_4         0xEF19 // #EDE0C8 
#define G2048_COL_8         0xF58F // #F2B179 
#define G2048_COL_16        0xF4AC // #F59563 
#define G2048_COL_32        0xF3EB // #F67C5F 
#define G2048_COL_64        0xF2E7 // #F65E3B 
#define G2048_COL_128       0xEE6E // #EDCF72 
#define G2048_COL_256       0xEE6C // #EDCC61 
#define G2048_COL_512       0xEE4A // #EDC850 
#define G2048_COL_1024      0xEE27 // #EDC53F 
#define G2048_COL_2048      0xEE05 // #EDC22E 
#define G2048_COL_SUPER     0x39C6 // #3C3A32 

typedef enum 
{
    DIR_NONE = 0,
    DIR_LEFT,
    DIR_RIGHT,
    DIR_UP,
    DIR_DOWN
} MoveDir;

static int grid[G2048_GRID_SIZE][G2048_GRID_SIZE];
static int gameScore;
static int gameBestScore;
static bool gameOver;
static bool gameWon;
static bool keepPlaying;
static bool needsRedraw;
static MoveDir pendingDir;
static uint32_t nextFrameAt;

static uint32_t gameNowMs(void) 
{
    return to_ms_since_boot(get_absolute_time());
}

static uint16_t getTileColor(int value) 
{
    switch (value) {
        case 0:    return G2048_COL_EMPTY;
        case 2:    return G2048_COL_2;
        case 4:    return G2048_COL_4;
        case 8:    return G2048_COL_8;
        case 16:   return G2048_COL_16;
        case 32:   return G2048_COL_32;
        case 64:   return G2048_COL_64;
        case 128:  return G2048_COL_128;
        case 256:  return G2048_COL_256;
        case 512:  return G2048_COL_512;
        case 1024: return G2048_COL_1024;
        case 2048: return G2048_COL_2048;
        default:   return G2048_COL_SUPER;
    }
}

static uint16_t getTileTextColor(int value) 
{
    return (value <= 4) ? G2048_COL_TEXT_DARK : G2048_COL_TEXT_LITE;
}

static void spawnTile(void) 
{
    int emptyR[G2048_GRID_SIZE * G2048_GRID_SIZE];
    int emptyC[G2048_GRID_SIZE * G2048_GRID_SIZE];
    int emptyCount = 0;

    for (int r = 0; r < G2048_GRID_SIZE; r++) {
        for (int c = 0; c < G2048_GRID_SIZE; c++) {
            if (grid[r][c] == 0) {
                emptyR[emptyCount] = r;
                emptyC[emptyCount] = c;
                emptyCount++;
            }
        }
    }

    if (emptyCount == 0) {
        return;
    }

    int pick = rand() % emptyCount;
    grid[emptyR[pick]][emptyC[pick]] = (rand() % 10 == 0) ? 4 : 2;
}

static bool hasAvailableMoves(void) 
{
    for (int r = 0; r < G2048_GRID_SIZE; r++) {
        for (int c = 0; c < G2048_GRID_SIZE; c++) {
            if (grid[r][c] == 0) {
                return true;
            }
            if (c + 1 < G2048_GRID_SIZE && grid[r][c] == grid[r][c + 1]) {
                return true;
            }
            if (r + 1 < G2048_GRID_SIZE && grid[r][c] == grid[r + 1][c]) {
                return true;
            }
        }
    }
    return false;
}

static bool slideAndMergeLine(int line[G2048_GRID_SIZE]) 
{
    int original[G2048_GRID_SIZE];
    int packed[G2048_GRID_SIZE] = { 0, 0, 0, 0 };
    int writeIdx = 0;

    for (int i = 0; i < G2048_GRID_SIZE; i++) {
        original[i] = line[i];
        if (line[i] != 0) {
            packed[writeIdx++] = line[i];
        }
    }

    for (int i = 0; i < G2048_GRID_SIZE - 1; i++) {
        if (packed[i] != 0 && packed[i] == packed[i + 1]) {
            packed[i] *= 2;
            packed[i + 1] = 0;
            
            gameScore += packed[i];
            if (gameScore > gameBestScore) {
                gameBestScore = gameScore;
            }
            if (packed[i] == 2048 && !gameWon) {
                gameWon = true;
            }
            i++;
        }
    }

    writeIdx = 0;
    for (int i = 0; i < G2048_GRID_SIZE; i++) {
        line[i] = 0;
    }
    for (int i = 0; i < G2048_GRID_SIZE; i++) {
        if (packed[i] != 0) {
            line[writeIdx++] = packed[i];
        }
    }

    for (int i = 0; i < G2048_GRID_SIZE; i++) {
        if (line[i] != original[i]) {
            return true;
        }
    }
    return false;
}

static bool moveBoard(MoveDir dir) 
{
    bool moved = false;
    int line[G2048_GRID_SIZE];

    if (dir == DIR_LEFT) {
        for (int r = 0; r < G2048_GRID_SIZE; r++) {
            for (int c = 0; c < G2048_GRID_SIZE; c++) {
                line[c] = grid[r][c];
            }
            if (slideAndMergeLine(line)) {
                moved = true;
            }
            for (int c = 0; c < G2048_GRID_SIZE; c++) {
                grid[r][c] = line[c];
            }
        }
    } else if (dir == DIR_RIGHT) {
        for (int r = 0; r < G2048_GRID_SIZE; r++) {
            for (int c = 0; c < G2048_GRID_SIZE; c++) {
                line[c] = grid[r][G2048_GRID_SIZE - 1 - c];
            }
            if (slideAndMergeLine(line)) {
                moved = true;
            }
            for (int c = 0; c < G2048_GRID_SIZE; c++) {
                grid[r][G2048_GRID_SIZE - 1 - c] = line[c];
            }
        }
    } else if (dir == DIR_UP) {
        for (int c = 0; c < G2048_GRID_SIZE; c++) {
            for (int r = 0; r < G2048_GRID_SIZE; r++) {
                line[r] = grid[r][c];
            }
            if (slideAndMergeLine(line)) {
                moved = true;
            }
            for (int r = 0; r < G2048_GRID_SIZE; r++) {
                grid[r][c] = line[r];
            }
        }
    } else if (dir == DIR_DOWN) {
        for (int c = 0; c < G2048_GRID_SIZE; c++) {
            for (int r = 0; r < G2048_GRID_SIZE; r++) {
                line[r] = grid[G2048_GRID_SIZE - 1 - r][c];
            }
            if (slideAndMergeLine(line)) {
                moved = true;
            }
            for (int r = 0; r < G2048_GRID_SIZE; r++) {
                grid[G2048_GRID_SIZE - 1 - r][c] = line[r];
            }
        }
    }
    return moved;
}

static void drawTile(int r, int c) 
{
    int value = grid[r][c];
    int tileX = G2048_BOARD_X + G2048_TILE_GAP + c * (G2048_TILE_SIZE + G2048_TILE_GAP);
    int tileY = G2048_BOARD_Y + G2048_TILE_GAP + r * (G2048_TILE_SIZE + G2048_TILE_GAP);
    uint16_t bgColor = getTileColor(value);

    GFX_fillRect(tileX, tileY, G2048_TILE_SIZE, G2048_TILE_SIZE, bgColor);

    if (value == 0) {
        return;
    }

    char buf[8];
    int len = snprintf(buf, sizeof(buf), "%d", value);
    uint8_t textSize = (value >= 1000) ? 1 : 2;
    int textW = len * 6 * textSize - textSize;
    int textH = 7 * textSize;
    int textX = tileX + (G2048_TILE_SIZE - textW) / 2;
    int textY = tileY + (G2048_TILE_SIZE - textH) / 2;

    GFX_setTextColor(getTileTextColor(value));
    GFX_setTextBack(bgColor);
    GFX_setTextSize(textSize);
    GFX_setCursor(textX, textY);
    GFX_printf("%s", buf);
}

static void drawBoardScene(void)
{
    GFX_fillScreen(G2048_COL_BG);

    GFX_fillRect(0, 0, G2048_SCREEN_W, G2048_HUD_H, BLACK);
    GFX_setTextColor(WHITE);
    GFX_setTextBack(BLACK);
    GFX_setTextSize(1);
    GFX_setCursor(4, 5);
    GFX_printf("2048  SCORE %d  BEST %d", gameScore, gameBestScore);

    GFX_fillRect(G2048_BOARD_X, G2048_BOARD_Y,
                G2048_BOARD_SIZE, G2048_BOARD_SIZE, G2048_COL_BOARD);

    for (int r = 0; r < G2048_GRID_SIZE; r++) {
        for (int c = 0; c < G2048_GRID_SIZE; c++) {
            drawTile(r, c);
        }
    }
}

static void renderGame(void)
{
    drawBoardScene();
    GFX_flush();
}

static void renderOverlay(const char *title, const char *hint, uint16_t accentColor)
{
    drawBoardScene();

    const int boxX = 45;
    const int boxY = 91;
    const int boxW = 230;
    const int boxH = 58;

    GFX_fillRect(boxX, boxY, boxW, boxH, BLACK);
    GFX_drawRect(boxX, boxY, boxW, boxH, accentColor);

    int titleW = (int)strlen(title) * 12 - 2;
    GFX_setTextColor(accentColor);
    GFX_setTextBack(BLACK);
    GFX_setTextSize(2);
    GFX_setCursor((G2048_SCREEN_W - titleW) / 2, boxY + 9);
    GFX_printf("%s", title);

    int hintW = (int)strlen(hint) * 6 - 1;
    GFX_setTextColor(WHITE);
    GFX_setTextSize(1);
    GFX_setCursor((G2048_SCREEN_W - hintW) / 2, boxY + 37);
    GFX_printf("%s", hint);

    GFX_flush();
}

static void resetGame(void)
{
    for (int r = 0; r < G2048_GRID_SIZE; r++) {
        for (int c = 0; c < G2048_GRID_SIZE; c++) {
            grid[r][c] = 0;
        }
    }

    gameScore = 0;
    gameOver = false;
    gameWon = false;
    keepPlaying = false;
    pendingDir = DIR_NONE;
    needsRedraw = false;

    spawnTile();
    spawnTile();

    nextFrameAt = gameNowMs();
}

void game2048Init(void)
{
    srand(time_us_32());
    resetGame();
    renderGame();
}

int game2048Read(void)
{
    if (btnPressed(BTN_B) || btnPressed(BTN_MENU) || btnPressed(BTN_MENU2)) {
    return 1;
    }

    if (gameOver) {
        if (btnPressed(BTN_A)) {
            resetGame();
            renderGame();
        }
        return 0;
    }

    if (gameWon && !keepPlaying) {
        if (btnPressed(BTN_A)) {
            keepPlaying = true;
            needsRedraw = true;
        }
        return 0;
    }

    if (btnPressed(BTN_LEFT)) {
        pendingDir = DIR_LEFT;
    } else if (btnPressed(BTN_RIGHT)) {
        pendingDir = DIR_RIGHT;
    } else if (btnPressed(BTN_UP)) {
        pendingDir = DIR_UP;
    } else if (btnPressed(BTN_DOWN)) {
        pendingDir = DIR_DOWN;
    }

    return 0;
}

void game2048Update(void)
{
    if (gameOver) {
        return;
    }

    uint32_t now = gameNowMs();
    if ((int32_t)(now - nextFrameAt) < 0) {
        return;
    }
    nextFrameAt = now + G2048_FRAME_DELAY;

    if (pendingDir != DIR_NONE && (!gameWon || keepPlaying)) {
        MoveDir dir = pendingDir;
        pendingDir = DIR_NONE;

        if (moveBoard(dir)) {
            spawnTile();
            if (!hasAvailableMoves()) {
                gameOver = true;
            }
            needsRedraw = true;
        }
    }

    if (!needsRedraw) {
        return;
    }
    needsRedraw = false;

    if (gameOver) {
        renderOverlay("GAME OVER!", "A: RETRY   B: MENU", RED);
    } else if (gameWon && !keepPlaying) {
        renderOverlay("YOU WIN!", "A: CONTINUE  B: MENU", GOLD);
    } else {
        renderGame();
    }
}