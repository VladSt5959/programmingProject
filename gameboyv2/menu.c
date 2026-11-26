#include "menu.h"

#include "colors.h"
#include "gfx.h"
#include "libs/ownLibs/btnReader.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define MENU_COLUMNS 4
#define MENU_ROWS 2
#define MENU_ITEMS 8

#define SCREEN_W 320
#define SCREEN_H 240
#define HEADER_H 32
#define CELL_W (SCREEN_W / MENU_COLUMNS)
#define CELL_H ((SCREEN_H - HEADER_H) / MENU_ROWS)
#define BOX_W 78
#define BOX_H 70
#define ICON_SCALE 3
#define ICON_SIZE (16 * ICON_SCALE)
#define LABEL_H 8
#define ITEM_CONTENT_H (ICON_SIZE + 2 + LABEL_H)

static int currentMenuIndex;
static int previousMenuIndex;
static int batteryPercent = -1;
static int lastDrawnBatteryPercent = -2;

static const uint16_t *const menuIcons[MENU_ITEMS] = {
    snake_game_16x16,
    mario_16x16,
    race_16x16,
    settings_icon_16x16,
    tetris_game_16x16,
    pacman_16x16,
    game2048_16x16,
clock_icon_16x16};

static const char *const menuLabels[MENU_ITEMS] = {
    "SNAKE", "MARIO", "RACE", "SETTINGS",
    "TETRIS", "PACMAN", "2048" , "CLOCK"};

static int cellX(int col)
{
    return col * CELL_W;
}

static int cellY(int row)
{
    return HEADER_H + row * CELL_H;
}

static int boxX(int col)
{
    return cellX(col) + (CELL_W - BOX_W) / 2;
}

static int boxY(int row)
{
    return cellY(row) + (CELL_H - BOX_H) / 2;
}

static void drawBatteryStatus(void)
{
    if (lastDrawnBatteryPercent == batteryPercent)
    {
        return;
    }

    GFX_fillRect(218, 2, 100, 28, PURPLE);
    GFX_setTextColor(WHITE);
    GFX_setTextBack(BLACK);
    GFX_setTextSize(1);
    GFX_setCursor(226, 12);
    if (batteryPercent < 0)
    {
        GFX_printf("BAT 87%%");
    }
    else
    {
        GFX_printf("BAT %d%%", batteryPercent);
    }

    lastDrawnBatteryPercent = batteryPercent;
    GFX_flush();
}

static void drawMenuItem(int index)
{
    int row = index / MENU_COLUMNS;
    int col = index % MENU_COLUMNS;
    int imageX = cellX(col) + (CELL_W - ICON_SIZE) / 2;
    int imageY = cellY(row) + (CELL_H - ITEM_CONTENT_H) / 2;
    int labelWidth = (int)strlen(menuLabels[index]) * 6;
    int labelX = cellX(col) + (CELL_W - labelWidth) / 2;
    int labelY = imageY + ICON_SIZE + 2;

    draw_image_scaled(imageX, imageY, 16, 16, ICON_SCALE, menuIcons[index]);
    GFX_setTextColor(WHITE);
    GFX_setTextBack(BLACK);
    GFX_setTextSize(1);
    GFX_setCursor(labelX, labelY);
    GFX_printf("%s", menuLabels[index]);
}

static int wrap(int value, int limit)
{
    if (value < 0)
        return limit - 1;
    if (value >= limit)
        return 0;
    return value;
}

static void moveSelection(int dx, int dy)
{
    int start = currentMenuIndex;
    int candidate = start;

   do
    {
        int row = candidate / MENU_COLUMNS;
        int col = candidate % MENU_COLUMNS;
        row = wrap(row + dy, MENU_ROWS);
        col = wrap(col + dx, MENU_COLUMNS);
        candidate = row * MENU_COLUMNS + col;
    } while (candidate >= MENU_ITEMS && candidate != start);

    currentMenuIndex = candidate;
}

void menuInit(void)
{
    currentMenuIndex = 0;
    previousMenuIndex = 0;
    lastDrawnBatteryPercent = -2;

    GFX_clearScreen();
    GFX_fillScreen(BLACK);

    GFX_fillRect(0, 0, SCREEN_W, HEADER_H, 0xF81F);
    GFX_fillRect(4, 4, SCREEN_W - 8, HEADER_H - 8, BLACK);
    GFX_setTextColor(CYAN);
    GFX_setTextBack(BLACK);
    GFX_setTextSize(2);
    GFX_setCursor(8, 8);
    GFX_printf("SELECT MISSION");

    for (int i = 0; i < MENU_ITEMS; i++)
    {
        drawMenuItem(i);
    }
    GFX_fillRect(3 * CELL_W, HEADER_H + 4,
                 1, SCREEN_H - HEADER_H - 8, CYAN);

    GFX_drawRect(boxX(0), boxY(0), BOX_W, BOX_H, PURPLE);
    GFX_flush();
    drawBatteryStatus();
}

int menuRead(void)
{
    if (btnPressed(BTN_LEFT))
    {
        previousMenuIndex = currentMenuIndex;
        moveSelection(-1, 0);
        return -2;
    }
    if (btnPressed(BTN_RIGHT))
    {
        previousMenuIndex = currentMenuIndex;
        moveSelection(1, 0);
        return -2;
    }
    if (btnPressed(BTN_UP))
    {
        previousMenuIndex = currentMenuIndex;
        moveSelection(0, -1);
        return -2;
    }
    if (btnPressed(BTN_DOWN))
    {
        previousMenuIndex = currentMenuIndex;
        moveSelection(0, 1);
        return -2;
    }
    if (btnPressed(BTN_A) || btnPressed(BTN_MENU))
    {
        return currentMenuIndex;
    }

    return -1;
}

void menuUpdate(void)
{
    if (previousMenuIndex != currentMenuIndex)
    {
        int oldRow = previousMenuIndex / MENU_COLUMNS;
        int oldCol = previousMenuIndex % MENU_COLUMNS;
        int newRow = currentMenuIndex / MENU_COLUMNS;
        int newCol = currentMenuIndex % MENU_COLUMNS;

        GFX_drawRect(boxX(oldCol), boxY(oldRow), BOX_W, BOX_H, BLACK);
        GFX_drawRect(boxX(newCol), boxY(newRow), BOX_W, BOX_H, PURPLE);
        previousMenuIndex = currentMenuIndex;
        GFX_flush();
    }

    drawBatteryStatus();
}

void menuTempSetBatteryPercent(int percent)
{
    if (percent < 0)
    {
        batteryPercent = -1;
    }
    else if (percent > 100)
    {
        batteryPercent = 100;
    }
    else
    {
        batteryPercent = percent;
    }
}
