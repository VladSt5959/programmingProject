#include "snake.h"
#include "colors.h"
#include "gfx.h"

#include "pico/stdlib.h"
#include <stdlib.h>
#include <stdlib.h>

#define BTN_RELOAD  2
#define BTN_LEFT    3
#define BTN_RIGHT   4
#define BTN_UP      5
#define BTN_DOWN    6

#define SIZE 20
#define MAX_LENGTH 100

int snakeLength = 5;
int snakeDirX = 1 , snakeDirY = 0;
int headX = 200 , headY = 20;
int appleX, appleY;

int snakeX[MAX_LENGTH];
int snakeY[MAX_LENGTH];

void snakeInit(){
    GFX_clearScreen();
    GFX_fillScreen(BLACK);
    appleX = (rand() % 16) * SIZE;
    appleY = (rand() % 12) * SIZE;    
    GFX_fillRect(appleX ,appleY,SIZE,SIZE,RED);
    snakeX[0] = 100;
    snakeY[0] = 100;

    // body after head
    for (int i = 1; i < snakeLength; i++)
    {
        snakeX[i] = snakeX[i - 1] - SIZE;
        snakeY[i] = snakeY[i - 1];
    }



}

void snakeRead()
{
    if (!gpio_get(BTN_UP) && snakeDirY != 1)
    {
        snakeDirX = 0;snakeDirY = -1;
    }
    if (!gpio_get(BTN_DOWN) && snakeDirY != -1)
    {
        snakeDirX = 0;snakeDirY = 1;
    }
    if (!gpio_get(BTN_LEFT) && snakeDirX != 1)
    {
        snakeDirX = -1;snakeDirY = 0;
    }
    if (!gpio_get(BTN_RIGHT) && snakeDirX != -1)
    {
        snakeDirX = 1;snakeDirY = 0;
    }
}

void snakeUpdate()
{
    // body to head
    for (int i = snakeLength - 1; i > 0; i--)
    {
        snakeX[i] = snakeX[i - 1];
        snakeY[i] = snakeY[i - 1];
    }

    //  head
    snakeX[0] += snakeDirX * SIZE;
    snakeY[0] += snakeDirY * SIZE;
     for (int i = 0; i < snakeLength; i++)
    {
        GFX_fillRect(snakeX[i],snakeY[i],SIZE,SIZE,PURPLE);
    }

    sleep_ms(150);
}

void appleUpdate(){
    GFX_fillRect(appleX ,appleY,SIZE,SIZE,BLACK);
    appleX = (rand() % 16) * SIZE;
    appleY = (rand() % 12) * SIZE;
    GFX_fillRect(appleX ,appleY,SIZE,SIZE,RED);
}