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

#define TFT_DC    20
#define TFT_CS    17
#define TFT_RST   21
#define TFT_SCK   18
#define TFT_MOSI  19

int state = 1; // 1 = Menu, 2 = Snake, 3 = Settings, 4 = Platformer (Mario), 5 = Extra placeholder

void initAll(){
    stdio_init_all();
    initBtns();
    initPhotoresistor();
    LCD_setPins(TFT_DC, TFT_CS, TFT_RST, TFT_SCK, TFT_MOSI);
    LCD_initDisplay();
    LCD_setRotation(3);
    GFX_createFramebuf();
    GFX_fillScreen(BLACK);
}

int main()
{
    initAll();
    state = 1;
    menuInit();

    
    while (true)
    {
        readBtns();
        printPhotoresistor();
        settingsService();
        

        if (state == 1) {
            int action = menuRead();
            menuUpdate();
            if (action == 0) {
                state = 2;
                snakeInit();
            } else if (action == 1) {
                state = 4;
                platformerInit();
            } else if (action == 2) {
                state = 3;
                settingsInit();
            } else if (action == 3) {
                // Extra slot / placeholder action
                state = 1;
                menuInit();
            }
        } else if (state == 2) {
            int exitToMenu = snakeRead();
            if (exitToMenu) {
                state = 1;
                menuInit();
            } else {
                snakeUpdate();
            }
        } else if (state == 3) {
            int exitToMenu = settingsRead();
            if (exitToMenu) {
                state = 1;
                menuInit();
            } else {
                settingsUpdate();
            }
        } else if (state == 4) {
            int exitToMenu = platformerRead();
            if (exitToMenu) {
                state = 1;
                menuInit();
            } else {
                platformerUpdate();
            }
        }

        tight_loop_contents();
        sleep_ms(10);
    }
}
