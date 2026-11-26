#include "menu.h"
#include "colors.h"
#include "gfx.h"
#include "pico/stdlib.h"
#include "libs/ownLibs/btnReader.h"

static int currentMenuIndex = 0; // 0: Snake, 1: Mario, 2: Settings, 3: Tetris 
static int prevMenuIndex = 0;
#define MENU_COLUMNS 3
#define MENU_ITEMS   6
#define BOX_WIDTH    98
#define BOX_HEIGHT   98

static const int boxX[MENU_COLUMNS] = { 4, 111, 218 };
static const int boxY[2] = { 36, 136 };
static const int iconX[MENU_COLUMNS] = { 21, 128, 235 };
static const int iconY[2] = { 50, 150 };
static const int labelY[2] = { 118, 218 };

void menuInit(void)
{
    currentMenuIndex = 0;
    prevMenuIndex = 0;

    GFX_clearScreen();
    GFX_fillScreen(BLACK);

    GFX_fillRect(0, 0, 320, 32, 0xF81F);
    GFX_fillRect(4, 4, 312, 24, BLACK);
    GFX_setTextColor(0x07FF);
    GFX_setTextBack(BLACK);
    GFX_setTextSize(2);
    GFX_setCursor(16, 8);
    GFX_printf("SELECT MISSION");

    GFX_setTextColor(WHITE);
    GFX_setTextSize(1);

    // Row 1
    draw_image_scaled(iconX[0], iconY[0], 16, 16, 4, tetris_block_16x16);
    GFX_setCursor(40, labelY[0]);
    GFX_printf("SNAKE");

    draw_image_scaled(iconX[1], iconY[0], 16, 16, 4, mario_16x16);
    GFX_setCursor(147, labelY[0]);
    GFX_printf("MARIO");

    draw_image_scaled(iconX[2], iconY[0], 16, 16, 4, settings_gear_16x16);
    GFX_setCursor(251, labelY[0]);
    GFX_printf("SETTINGS");

    // Row 2
    draw_image_scaled(iconX[0], iconY[1], 16, 16, 4, coin_16x16);
    GFX_setCursor(40, labelY[1]);
    GFX_printf("EXTRA");

    // Reserved icon slot 5
    GFX_fillRect(iconX[1], iconY[1], 64, 64, 0x2104);
    GFX_drawRect(iconX[1], iconY[1], 64, 64, 0x07FF);
    GFX_setCursor(144, labelY[1]);
    GFX_printf("NEW 1");

    // Reserved icon slot 6
    GFX_fillRect(iconX[2], iconY[1], 64, 64, 0x2104);
    GFX_drawRect(iconX[2], iconY[1], 64, 64, 0x07FF);
    GFX_setCursor(251, labelY[1]);
    GFX_printf("NEW 2");

    GFX_drawRect(boxX[0], boxY[0], BOX_WIDTH, BOX_HEIGHT, 0x07FF);
    GFX_flush();
}

int menuRead(void)
{
    int row = currentMenuIndex / MENU_COLUMNS;
    int col = currentMenuIndex % MENU_COLUMNS;

    if (btnPressed(BTN_LEFT)) {
        prevMenuIndex = currentMenuIndex;
        col = (col + MENU_COLUMNS - 1) % MENU_COLUMNS;
        currentMenuIndex = row * MENU_COLUMNS + col;
        return -2;
    }

    if (btnPressed(BTN_RIGHT)) {
        prevMenuIndex = currentMenuIndex;
        col = (col + 1) % MENU_COLUMNS;
        currentMenuIndex = row * MENU_COLUMNS + col;
        return -2;
    }

    if (btnPressed(BTN_UP)) {
        prevMenuIndex = currentMenuIndex;
        row = (row + 1) % 2;
        currentMenuIndex = row * MENU_COLUMNS + col;
        return -2;
    }

    if (btnPressed(BTN_DOWN)) {
        prevMenuIndex = currentMenuIndex;
        row = (row + 1) % 2;
        currentMenuIndex = row * MENU_COLUMNS + col;
        return -2;
    }

    if (btnPressed(BTN_A) || btnPressed(BTN_MENU)) {
        return currentMenuIndex;
    }

    return -1;
}

void menuUpdate(void)
{
    if (prevMenuIndex == currentMenuIndex) {
        return;
    }

    int oldRow = prevMenuIndex / MENU_COLUMNS;
    int oldCol = prevMenuIndex % MENU_COLUMNS;
    int newRow = currentMenuIndex / MENU_COLUMNS;
    int newCol = currentMenuIndex % MENU_COLUMNS;

    GFX_drawRect(boxX[oldCol], boxY[oldRow],
                 BOX_WIDTH, BOX_HEIGHT, BLACK);
    GFX_drawRect(boxX[oldCol] + 1, boxY[oldRow] + 1,
                 BOX_WIDTH - 2, BOX_HEIGHT - 2, BLACK);

    GFX_drawRect(boxX[newCol], boxY[newRow],
                 BOX_WIDTH, BOX_HEIGHT, 0x07FF);

    prevMenuIndex = currentMenuIndex;
    GFX_flush();
}