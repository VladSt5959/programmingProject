#include "pico/stdlib.h"
#include "pico/bootrom.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "gfx.h"
#include "font.h"
#include "gfxfont.h"

#include "ili9341.h"

#include "hardware/dma.h"
#include "colors.h"

#include "games/snake.h"

#define TFT_DC    20
#define TFT_CS    17
#define TFT_RST   21
#define TFT_SCK   18
#define TFT_MOSI  19
// #define TFT_MISO  16

#define BTN_RELOAD  2
#define BTN_LEFT    3
#define BTN_RIGHT   4
#define BTN_UP      5
#define BTN_DOWN    6



    int previousCounter = 0;
    int counter = 0;
    char text[2];
    int inMenu = 1;


static const uint16_t mario_16x16[16 * 16] = 
{
    PURPLE, PURPLE, PURPLE, RED,   RED,   RED,   RED,   RED,   PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE,
    PURPLE, PURPLE, RED,   RED,   RED,   RED,   RED,   RED,   RED,   RED,   RED,   PURPLE, PURPLE, PURPLE, PURPLE, PURPLE,
    PURPLE, PURPLE, BROWN, BROWN, BROWN, SKIN,  SKIN,  PURPLE, SKIN,  PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE,
    PURPLE, BROWN, SKIN,  BROWN, SKIN,  SKIN,  SKIN,  PURPLE, SKIN,  SKIN,  SKIN,  PURPLE, PURPLE, PURPLE, PURPLE, PURPLE,
    PURPLE, BROWN, SKIN,  BROWN, BROWN, SKIN,  SKIN,  SKIN,  PURPLE, SKIN,  SKIN,  SKIN,  PURPLE, PURPLE, PURPLE, PURPLE,
    PURPLE, BROWN, BROWN, SKIN,  SKIN,  SKIN,  SKIN,  PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE,
    PURPLE, PURPLE, PURPLE, SKIN,  SKIN,  SKIN,  SKIN,  SKIN,  SKIN,  SKIN,  PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE,
    PURPLE, PURPLE, RED,   RED,   BLUE,  RED,   RED,   RED,   PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE,
    PURPLE, RED,   RED,   RED,   BLUE,  RED,   RED,   BLUE,  RED,   RED,   RED,   PURPLE, PURPLE, PURPLE, PURPLE, PURPLE,
    RED,   RED,   RED,   RED,   BLUE,  BLUE,  BLUE,  BLUE,  RED,   RED,   RED,   RED,   PURPLE, PURPLE, PURPLE, PURPLE,
    SKIN,  SKIN,  RED,   BLUE,  YELLOW,BLUE,  BLUE,  YELLOW,BLUE,  RED,   SKIN,  SKIN,  PURPLE, PURPLE, PURPLE, PURPLE,
    SKIN,  SKIN,  SKIN,  BLUE,  BLUE,  BLUE,  BLUE,  BLUE,  BLUE,  SKIN,  SKIN,  SKIN,  PURPLE, PURPLE, PURPLE, PURPLE,
    SKIN,  SKIN,  BLUE,  BLUE,  BLUE,  BLUE,  BLUE,  BLUE,  BLUE,  BLUE,  SKIN,  SKIN,  PURPLE, PURPLE, PURPLE, PURPLE,
    PURPLE, PURPLE, BLUE,  BLUE,  BLUE,  PURPLE, PURPLE, BLUE,  BLUE,  BLUE,  PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE,
    PURPLE, BROWN, BROWN, BROWN, PURPLE, PURPLE, PURPLE, PURPLE, BROWN, BROWN, BROWN, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE,
    BROWN, BROWN, BROWN, BROWN, PURPLE, PURPLE, PURPLE, PURPLE, BROWN, BROWN, BROWN, BROWN, PURPLE, PURPLE, PURPLE, PURPLE
};

static const uint16_t coin_16x16[16 * 16] = {
    PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, GOLD,  GOLD,  GOLD,  GOLD,  PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE,
    PURPLE, PURPLE, PURPLE, GOLD,  GOLD,  YELLOW, YELLOW, YELLOW, GOLD,  GOLD,  PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE,
    PURPLE, PURPLE, GOLD,  YELLOW, YELLOW, WHITE,  WHITE,  YELLOW, YELLOW, GOLD,  GOLD,  PURPLE, PURPLE, PURPLE, PURPLE, PURPLE,
    PURPLE, GOLD,  YELLOW, WHITE,  WHITE,  YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, GOLD,  PURPLE, PURPLE, PURPLE, PURPLE, PURPLE,
    PURPLE, GOLD,  YELLOW, WHITE,  YELLOW, GOLD,   GOLD,   YELLOW, YELLOW, YELLOW, GOLD,  PURPLE, PURPLE, PURPLE, PURPLE, PURPLE,
    GOLD,  YELLOW, YELLOW, YELLOW, GOLD,  PURPLE, PURPLE, GOLD,   YELLOW, YELLOW, GOLD,  GOLD,  PURPLE, PURPLE, PURPLE, PURPLE,
    GOLD,  YELLOW, YELLOW, YELLOW, GOLD,  PURPLE, PURPLE, GOLD,   YELLOW, YELLOW, GOLD,  GOLD,  PURPLE, PURPLE, PURPLE, PURPLE,
    GOLD,  YELLOW, YELLOW, YELLOW, GOLD,  PURPLE, PURPLE, GOLD,   YELLOW, YELLOW, GOLD,  GOLD,  PURPLE, PURPLE, PURPLE, PURPLE,
    GOLD,  YELLOW, YELLOW, YELLOW, GOLD,  PURPLE, PURPLE, GOLD,   YELLOW, YELLOW, GOLD,  GOLD,  PURPLE, PURPLE, PURPLE, PURPLE,
    GOLD,  YELLOW, YELLOW, YELLOW, GOLD,  PURPLE, PURPLE, GOLD,   YELLOW, YELLOW, GOLD,  GOLD,  PURPLE, PURPLE, PURPLE, PURPLE,
    GOLD,  YELLOW, YELLOW, YELLOW, GOLD,  PURPLE, PURPLE, GOLD,   YELLOW, YELLOW, GOLD,  GOLD,  PURPLE, PURPLE, PURPLE, PURPLE,
    PURPLE, GOLD,  YELLOW, YELLOW, YELLOW, GOLD,   GOLD,   YELLOW, YELLOW, YELLOW, GOLD,  PURPLE, PURPLE, PURPLE, PURPLE, PURPLE,
    PURPLE, GOLD,  YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, GOLD,  PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE,
    PURPLE, PURPLE, GOLD,  YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, GOLD,  PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE,
    PURPLE, PURPLE, PURPLE, GOLD,  GOLD,  YELLOW, YELLOW, GOLD,  GOLD,  PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE,
    PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, GOLD,  GOLD,  PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE
};

static const uint16_t tetris_block_16x16[16 * 16] = {
    WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, PURPLE,
    WHITE, CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  DARKBLUE, PURPLE,
    WHITE, CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  DARKBLUE, PURPLE,
    WHITE, CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  DARKBLUE, PURPLE,
    WHITE, CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  DARKBLUE, PURPLE,
    WHITE, CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  DARKBLUE, PURPLE,
    WHITE, CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  DARKBLUE, PURPLE,
    WHITE, CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  DARKBLUE, PURPLE,
    WHITE, CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  DARKBLUE, PURPLE,
    WHITE, CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  DARKBLUE, PURPLE,
    WHITE, CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  DARKBLUE, PURPLE,
    WHITE, CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  DARKBLUE, PURPLE,
    WHITE, CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  DARKBLUE, PURPLE,
    WHITE, CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  CYAN,  DARKBLUE, PURPLE,
    WHITE, DARKBLUE, DARKBLUE, DARKBLUE, DARKBLUE, DARKBLUE, DARKBLUE, DARKBLUE, DARKBLUE, DARKBLUE, DARKBLUE, DARKBLUE, DARKBLUE, DARKBLUE, DARKBLUE, PURPLE,
    PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE
};

static const uint16_t settings_gear_16x16[16 * 16] = {
    PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, GRAY,  GRAY,  GRAY,  GRAY,  PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE,
    PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, GRAY,  LIGHTGRAY, LIGHTGRAY, GRAY, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE,
    PURPLE, PURPLE, GRAY,  GRAY,  PURPLE, GRAY,  LIGHTGRAY, LIGHTGRAY, GRAY, PURPLE, GRAY,  GRAY,  PURPLE, PURPLE, PURPLE, PURPLE,
    PURPLE, PURPLE, GRAY,  LIGHTGRAY, GRAY_LIGHT, GRAY_LIGHT, GRAY_LIGHT, GRAY_LIGHT, GRAY_LIGHT, GRAY_LIGHT, LIGHTGRAY, GRAY, PURPLE, PURPLE, PURPLE, PURPLE,
    PURPLE, PURPLE, PURPLE, GRAY_LIGHT, GRAY_LIGHT, DARKGRAY, DARKGRAY, DARKGRAY, DARKGRAY, GRAY_LIGHT, GRAY_LIGHT, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE,
    GRAY,  GRAY,  GRAY,  GRAY_LIGHT, DARKGRAY, GRAY_LIGHT, GRAY_LIGHT, GRAY_LIGHT, GRAY_LIGHT, DARKGRAY, GRAY_LIGHT, GRAY,  GRAY,  GRAY,  PURPLE, PURPLE,
    GRAY,  LIGHTGRAY, LIGHTGRAY, GRAY_LIGHT, DARKGRAY, GRAY_LIGHT, PURPLE, PURPLE, GRAY_LIGHT, DARKGRAY, GRAY_LIGHT, LIGHTGRAY, LIGHTGRAY, GRAY, PURPLE, PURPLE,
    GRAY,  LIGHTGRAY, LIGHTGRAY, GRAY_LIGHT, DARKGRAY, GRAY_LIGHT, PURPLE, PURPLE, GRAY_LIGHT, DARKGRAY, GRAY_LIGHT, LIGHTGRAY, LIGHTGRAY, GRAY, PURPLE, PURPLE,
    GRAY,  GRAY,  GRAY,  GRAY_LIGHT, DARKGRAY, GRAY_LIGHT, GRAY_LIGHT, GRAY_LIGHT, GRAY_LIGHT, DARKGRAY, GRAY_LIGHT, GRAY,  GRAY,  GRAY,  PURPLE, PURPLE,
    PURPLE, PURPLE, PURPLE, GRAY_LIGHT, GRAY_LIGHT, DARKGRAY, DARKGRAY, DARKGRAY, DARKGRAY, GRAY_LIGHT, GRAY_LIGHT, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE,
    PURPLE, PURPLE, GRAY,  LIGHTGRAY, GRAY_LIGHT, GRAY_LIGHT, GRAY_LIGHT, GRAY_LIGHT, GRAY_LIGHT, GRAY_LIGHT, LIGHTGRAY, GRAY, PURPLE, PURPLE, PURPLE, PURPLE,
    PURPLE, PURPLE, GRAY,  GRAY,  PURPLE, GRAY,  LIGHTGRAY, LIGHTGRAY, GRAY, PURPLE, GRAY,  GRAY,  PURPLE, PURPLE, PURPLE, PURPLE,
    PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, GRAY,  LIGHTGRAY, LIGHTGRAY, GRAY, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE,
    PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, GRAY,  GRAY,  GRAY,  GRAY,  PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE,
    PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE,
    PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE
};

void initAll(){
//display init
    stdio_init_all();

    LCD_setPins(
        TFT_DC,
        TFT_CS,
        TFT_RST,
        TFT_SCK,
        TFT_MOSI
    );

    LCD_initDisplay();
    LCD_setRotation(3);
    GFX_createFramebuf();
    GFX_fillScreen(PURPLE);

    //btns init 
    gpio_init(BTN_LEFT);gpio_set_dir(BTN_LEFT, GPIO_IN);gpio_pull_up(BTN_LEFT);
    gpio_init(BTN_RIGHT);gpio_set_dir(BTN_RIGHT, GPIO_IN);gpio_pull_up(BTN_RIGHT);
    gpio_init(BTN_UP);gpio_set_dir(BTN_UP, GPIO_IN);gpio_pull_up(BTN_UP);
    gpio_init(BTN_DOWN);gpio_set_dir(BTN_DOWN, GPIO_IN);gpio_pull_up(BTN_DOWN); 
    gpio_init(BTN_RELOAD);gpio_set_dir(BTN_RELOAD, GPIO_IN);gpio_pull_up(BTN_RELOAD); 
}

void readBtns(){
      //read input from btns 
            //make it as separate function later 
         if (!gpio_get(BTN_UP))
        {
            counter++;
            if(counter >= 4){
                counter = 0;
            }
            GFX_fillRect(40, 60, 80, 10, BLACK);
            GFX_setTextColor(WHITE);
            GFX_setTextBack(BLACK);
            GFX_setTextSize(1);
            GFX_setCursor(40, 60);
            snprintf(text, sizeof(text), "%d", counter);
            GFX_printf(text);
            GFX_flush();
            
         }  // need to deal with btn debounce for all of em at a time for future 
         if (!gpio_get(BTN_DOWN))
        {
            counter--;
            if(counter < 0){
                counter = 3;
            }
            GFX_fillRect(40, 60, 80, 10, BLACK);
            GFX_setTextColor(WHITE);
            GFX_setTextBack(BLACK);
            GFX_setTextSize(1);
            GFX_setCursor(40, 60);
            snprintf(text, sizeof(text), "%d", counter);
            GFX_printf(text);
            GFX_flush();
        }  // need to deal with btn debounce for all of em at a time for future
        if (!gpio_get(BTN_RELOAD)) {
                 reset_usb_boot(0, 0);
        }
        if(!gpio_get(BTN_RIGHT) && counter== 0){
            snakeInit();
            inMenu = 0 ;
        }
}


int main()
{
    initAll();

    if(inMenu == 1){
          // text
    GFX_setTextColor(WHITE);
    GFX_setTextBack(BLACK);
    GFX_setTextSize(2);
    GFX_setCursor(20, 20);
    GFX_printf("Pico 2 W Game");

    // draw icons
    draw_image_scaled(20,  120,  16, 16, 4, coin_16x16);
    draw_image_scaled(90,  120,  16, 16, 4, tetris_block_16x16);
    draw_image_scaled(160, 120, 16, 16, 4, mario_16x16);
    draw_image_scaled(230, 120,  16, 16, 4, settings_gear_16x16);
    }

  
    // draw frame
    GFX_flush();



    while (true)
    {
        readBtns();
         
        if(inMenu == 1){
        
                // rect to see what option in main menu is choosen 
        int oldX = 20 + previousCounter * 70;
        int newX = 20 + counter * 70;
        GFX_drawRect(oldX, 120, 70, 70, PURPLE);
        GFX_drawRect(newX, 120, 70, 70, WHITE);
        previousCounter = counter;
        }
        else{
            snakeRead();
            snakeUpdate();
        }
    

        tight_loop_contents();

        sleep_ms(10);
    }
}

//    to enter bootsel just by clickig 1 btn 
//    if (!gpio_get(BTN_UP)) {
//                  reset_usb_boot(0, 0);
//         }


// just in case 

   // Rectangle
    // GFX_fillRect(
    //     10,
    //     10,
    //     100,
    //     40,
    //     YELLOW
    // );

       // // Some shapes
    // GFX_drawRect(
    //     10,
    //     100,
    //     150,
    //     70,
    //     GREEN
    // );

    // GFX_fillCircle(
    //     220,
    //     120,
    //     25,
    //     BLUE
    // );

