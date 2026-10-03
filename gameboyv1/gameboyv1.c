#include "pico/stdlib.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "gfx.h"
#include "font.h"
#include "gfxfont.h"

#include "ili9341.h"

#include "hardware/dma.h"

#define TFT_DC    20
#define TFT_CS    17
#define TFT_RST   21
#define TFT_SCK   18
#define TFT_MOSI  19



// #define TFT_MISO  16



#define BLACK   0x0000
#define WHITE   0xFFFF
#define RED     0xF800
#define GREEN   0x07E0
#define BLUE    0x001F
#define YELLOW  0xFFE0
#define PURPLE  0x8010


void draw_image(
    int x,
    int y,
    int width,
    int height,
    const uint16_t *image
)
{
    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            GFX_drawPixel(
                x + col,
                y + row,
                image[row * width + col]
            );
        }
    }
}




const uint16_t player[8 * 8] =
{
    BLACK,BLACK,RED,RED,RED,RED,BLACK,BLACK,
    BLACK,RED,RED,RED,RED,RED,RED,BLACK,
    RED,RED,WHITE,RED,RED,WHITE,RED,RED,
    RED,RED,RED,RED,RED,RED,RED,RED,
    RED,WHITE,RED,RED,RED,RED,WHITE,RED,
    RED,RED,WHITE,WHITE,WHITE,WHITE,RED,RED,
    BLACK,RED,RED,RED,RED,RED,RED,BLACK,
    BLACK,BLACK,RED,RED,RED,RED,BLACK,BLACK
};


int main()
{
    stdio_init_all();

    LCD_setPins(
        TFT_DC,
        TFT_CS,
        TFT_RST,
        TFT_SCK,
        TFT_MOSI
    );

    LCD_initDisplay();

    // landscape
    LCD_setRotation(3);

    GFX_createFramebuf();

    GFX_fillScreen(BLACK);


    // Rectangle
    GFX_fillRect(
        10,
        10,
        100,
        40,
        PURPLE
    );


    // Text
    GFX_setTextColor(WHITE);
    GFX_setTextBack(BLACK);

    GFX_setCursor(20, 70);
    GFX_printf("Pico 2 W Game");


    // Some shapes
    GFX_drawRect(
        10,
        100,
        150,
        70,
        GREEN
    );

    GFX_fillCircle(
        220,
        120,
        25,
        BLUE
    );


    // Sprite
    draw_image(
        100,
        180,
        8,
        8,
        player
    );


    // Send RAM framebuffer to TFT
    GFX_flush();


    while (true)
    {
        tight_loop_contents();
    }
}