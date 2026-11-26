#include "pico/stdlib.h"
#include "pico/bootrom.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "libs/ownLibs/btnReader.h"
#include "gfx.h"
#include "font.h"
#include "gfxfont.h"
#include "ili9341.h"
#include "hardware/dma.h"
#include "colors.h"

#include "menu.h"
#include "games/snake.h"
#include "games/platformer.h"
#include "settings.h"
#include "games/racing_game.h"
#include "games/tetris.h"
#include "games/pacman.h"
#include "games/game_2048.h"
#include "games/clock.h"

#define TFT_DC 20
#define TFT_CS 17
#define TFT_RST 21
#define TFT_SCK 18
#define TFT_MOSI 19

int state = 1; 
void initAll()
{
    stdio_init_all();
    initBtns();
    initPhotoresistor();
    LCD_setPins(TFT_DC, TFT_CS, TFT_RST, TFT_SCK, TFT_MOSI);
    LCD_initDisplay();
    initBacklightPWM();
    LCD_setRotation(3);
    GFX_createFramebuf();
    GFX_fillScreen(BLACK);
}

int main()
{
    initAll();
    state = 1;
    menuInit();
    clockAppStartup();


    while (true)
    {
        readBtns();
        printPhotoresistor();
        settingsService();

        if (state == 1)
        {
            int action = menuRead();
            menuUpdate();
            if (action == 0)
            {
                state = 2;
                snakeInit();
            }
            else if (action == 1)
            {
                state = 4;
                platformerInit();
            }
            else if (action == 2)
            {
                state = 5;
                settingsInit();
            }
            else if (action == 3)
            {
                state = 3;
                racingGameInit();
            }
            else if (action == 4)
            {
                state = 6;
                tetrisInit();
            }
            else if (action == 5)
            {
                state = 7;
                pacmanInit();
            }
            else if (action == 6)
            {
                state = 8;
                game2048Init();
            }
              else if (action == 7)
            {
                state = 9;
                clockAppInit();
            }
        }
        else if (state == 2)
        {

            int exitToMenu = snakeRead();
            if (exitToMenu)
            {
                state = 1;
                menuInit();
            }
            else
            {
                snakeUpdate();
            }
        }
        else if (state == 3)
        {
            int exitToMenu = settingsRead();
            if (exitToMenu)
            {
                state = 1;
                menuInit();
            }
            else
            {
                settingsUpdate();
            }
        }
        else if (state == 4)
        {
            int exitToMenu = platformerRead();
            if (exitToMenu)
            {
                state = 1;
                menuInit();
            }
            else
            {
                platformerUpdate();
            }
        }
        else if (state == 5)
        {
            int exitToMenu = racingGameRead();
            if (exitToMenu)
            {
                state = 1;
                menuInit();
            }
            else
            {
                racingGameUpdate();
            }
        }
        else if (state == 6)
        {
            int exitToMenu = tetrisRead();
            if (exitToMenu)
            {
                state = 1;
                menuInit();
            }
            else
            {
                tetrisUpdate();
            }
        }
         else if (state == 7)
        {
            int exitToMenu = pacmanRead();
            if (exitToMenu)
            {
                state = 1;
                menuInit();
            }
            else
            {
                pacmanUpdate();
            }
        }
           else if (state == 8)
        {
            int exitToMenu = game2048Read();
            if (exitToMenu)
            {
                state = 1;
                menuInit();
            }
            else
            {
              game2048Update();
            }
        }
         else if (state == 9)
        {
            int exitToMenu = clockAppRead();
            if (exitToMenu)
            {
                state = 1;
                menuInit();
            }
            else
            {
              clockAppUpdate();
            }
        }
    }

    tight_loop_contents();
    sleep_ms(10);
}
