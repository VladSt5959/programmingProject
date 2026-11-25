#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/rand.h"
#include "racing_game.h"
#include "ili9341.h"
#include "buttons.h"

static void render_game(void) {
    uint16_t color_road   = ili9341_rgb565(25, 25, 30);
    uint16_t color_hud    = ili9341_rgb565(10, 10, 15);
    uint16_t color_border = ili9341_rgb565(220, 220, 220);
    uint16_t color_car    = ili9341_rgb565(0, 220, 120);
    uint16_t color_obst   = ili9341_rgb565(235, 60, 60);
    uint16_t color_text   = ili9341_rgb565(255, 255, 255);

    /* Background and top HUD bar */
    ili9341_clear(color_road);
    ili9341_fill_rect(0, 0, SCREEN_W, HUD_HEIGHT, color_hud);

    /* Track top and bottom borders */
    ili9341_fill_rect(0, HUD_HEIGHT, SCREEN_W, BORDER_THICKNESS, color_border);
    ili9341_fill_rect(0, TRACK_BOTTOM, SCREEN_W, BORDER_THICKNESS, color_border);

    /* Player car */
    ili9341_fill_rect(car_x, car_y, CAR_WIDTH, CAR_HEIGHT, color_car);

    /* Obstacles */
    for (int i = 0; i < OBST_COUNT; i++) {
        ili9341_fill_rect(obst_x[i], obst_y[i], CAR_WIDTH, CAR_HEIGHT, color_obst);
    }

    /* HUD text */
    char hud_buf[32];
    snprintf(hud_buf, sizeof(hud_buf), "SCORE:%d", score);
    ili9341_draw_text(8, 5, hud_buf, color_text, 2);

    snprintf(hud_buf, sizeof(hud_buf), "BEST:%d", high_score);
    ili9341_draw_text(180, 5, hud_buf, color_text, 2);

    ili9341_flush();
}

static void show_game_over_screen(void) {
    uint16_t color_bg   = ili9341_rgb565(15, 10, 15);
    uint16_t color_red  = ili9341_rgb565(240, 50, 50);
    uint16_t color_text = ili9341_rgb565(255, 255, 255);
    uint16_t color_hint = ili9341_rgb565(0, 220, 120);

    ili9341_clear(color_bg);
    ili9341_draw_text(88, 55, "GAME OVER!", color_red, 4);

    char buf[32];
    snprintf(buf, sizeof(buf), "SCORE:%d", score);
    ili9341_draw_text(110, 115, buf, color_text, 2);

    snprintf(buf, sizeof(buf), "BEST:%d", high_score);
    ili9341_draw_text(110, 140, buf, color_text, 2);

    ili9341_draw_text(72, 185, "PRESS RESTART", color_hint, 2);
    ili9341_flush();

    /* Wait for release first, then a clean debounced press */
    while (button_restart_pressed()) {
        sleep_ms(10);
    }
    while (!button_restart_pressed()) {
        sleep_ms(10);
    }
    sleep_ms(30);
    reset_game();
}

int main(void) {
    stdio_init_all();
    buttons_init();
    ili9341_init();

    srand(get_rand_32());
    reset_game();

    while (true) {
        if (button_up_pressed()) {
            move_car(-CAR_STEP);
        }
        if (button_down_pressed()) {
            move_car(CAR_STEP);
        }

        check_collision();
        if (!isGameOver) {
            update_obstacles();
            check_collision();
        }

        if (isGameOver) {
            show_game_over_screen();
        } else {
            render_game();
            sleep_ms(game_speed);
        }
    }

    return 0;
}