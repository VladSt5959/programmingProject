#include <stddef.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "racing_game.h"
#include "ili9341.h"

static uint16_t framebuffer[SCREEN_W * SCREEN_H];

static void write_cmd(uint8_t cmd) {
    gpio_put(ILI9341_PIN_DC, 0);
    gpio_put(ILI9341_PIN_CS, 0);
    spi_write_blocking(ILI9341_SPI_PORT, &cmd, 1);
    gpio_put(ILI9341_PIN_CS, 1);
}

static void write_data(const uint8_t *data, size_t len) {
    gpio_put(ILI9341_PIN_DC, 1);
    gpio_put(ILI9341_PIN_CS, 0);
    spi_write_blocking(ILI9341_SPI_PORT, data, len);
    gpio_put(ILI9341_PIN_CS, 1);
}

static void write_byte(uint8_t val) {
    write_data(&val, 1);
}

void ili9341_init(void) {
    spi_init(ILI9341_SPI_PORT, ILI9341_SPI_BAUD);
    gpio_set_function(ILI9341_PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(ILI9341_PIN_MOSI, GPIO_FUNC_SPI);

    gpio_init(ILI9341_PIN_CS);
    gpio_set_dir(ILI9341_PIN_CS, GPIO_OUT);
    gpio_put(ILI9341_PIN_CS, 1);

    gpio_init(ILI9341_PIN_DC);
    gpio_set_dir(ILI9341_PIN_DC, GPIO_OUT);

    gpio_init(ILI9341_PIN_RST);
    gpio_set_dir(ILI9341_PIN_RST, GPIO_OUT);

    gpio_init(ILI9341_PIN_BL);
    gpio_set_dir(ILI9341_PIN_BL, GPIO_OUT);
    gpio_put(ILI9341_PIN_BL, 1);

    /* Hardware reset */
    gpio_put(ILI9341_PIN_RST, 0);
    sleep_ms(20);
    gpio_put(ILI9341_PIN_RST, 1);
    sleep_ms(120);

    /* Standard ILI9341 startup sequence */
    write_cmd(0x01); /* SWRESET */
    sleep_ms(120);

    write_cmd(0x11); /* SLPOUT */
    sleep_ms(120);

    write_cmd(0x3A); /* COLMOD: 16-bit RGB565 */
    write_byte(0x55);

    write_cmd(0x36); /* MADCTL: 0x28 = Landscape 320x240 BGR (use 0xE8 if upside down) */
    write_byte(0x28);

    write_cmd(0x29); /* DISPON */
    sleep_ms(20);
}

void ili9341_clear(uint16_t color) {
    for (int i = 0; i < SCREEN_W * SCREEN_H; i++) {
        framebuffer[i] = color;
    }
}

void ili9341_fill_rect(int x, int y, int w, int h, uint16_t color) {
    int x0 = x < 0 ? 0 : x;
    int y0 = y < 0 ? 0 : y;
    int x1 = (x + w) > SCREEN_W ? SCREEN_W : (x + w);
    int y1 = (y + h) > SCREEN_H ? SCREEN_H : (y + h);

    for (int py = y0; py < y1; py++) {
        uint16_t *row_ptr = &framebuffer[py * SCREEN_W];
        for (int px = x0; px < x1; px++) {
            row_ptr[px] = color;
        }
    }
}

/* Minimal 3x5 pixel block glyphs for HUD and Game Over messages */
static void get_glyph_3x5(char c, uint8_t out[5]) {
    uint8_t blank[5] = {0, 0, 0, 0, 0};
    for (int i = 0; i < 5; i++) out[i] = blank[i];

    const uint8_t *g = NULL;
    static const uint8_t digits[10][5] = {
        {7, 5, 5, 5, 7}, /* 0 */
        {2, 6, 2, 2, 7}, /* 1 */
        {7, 1, 7, 4, 7}, /* 2 */
        {7, 1, 7, 1, 7}, /* 3 */
        {5, 5, 7, 1, 1}, /* 4 */
        {7, 4, 7, 1, 7}, /* 5 */
        {7, 4, 7, 5, 7}, /* 6 */
        {7, 1, 1, 2, 2}, /* 7 */
        {7, 5, 7, 5, 7}, /* 8 */
        {7, 5, 7, 1, 7}  /* 9 */
    };

    if (c >= '0' && c <= '9') {
        g = digits[c - '0'];
    } else {
        static const uint8_t ch_A[5] = {2, 5, 7, 5, 5};
        static const uint8_t ch_B[5] = {6, 5, 6, 5, 6};
        static const uint8_t ch_C[5] = {7, 4, 4, 4, 7};
        static const uint8_t ch_D[5] = {6, 5, 5, 5, 6};
        static const uint8_t ch_E[5] = {7, 4, 6, 4, 7};
        static const uint8_t ch_G[5] = {7, 4, 5, 5, 7};
        static const uint8_t ch_M[5] = {5, 7, 5, 5, 5};
        static const uint8_t ch_O[5] = {7, 5, 5, 5, 7};
        static const uint8_t ch_P[5] = {7, 5, 7, 4, 4};
        static const uint8_t ch_R[5] = {6, 5, 6, 5, 5};
        static const uint8_t ch_S[5] = {7, 4, 7, 1, 7};
        static const uint8_t ch_T[5] = {7, 2, 2, 2, 2};
        static const uint8_t ch_V[5] = {5, 5, 5, 5, 2};
        static const uint8_t ch_colon[5] = {0, 2, 0, 2, 0};
        static const uint8_t ch_bang[5]  = {2, 2, 2, 0, 2};

        switch (c) {
            case 'A': g = ch_A; break;
            case 'B': g = ch_B; break;
            case 'C': g = ch_C; break;
            case 'D': g = ch_D; break;
            case 'E': g = ch_E; break;
            case 'G': g = ch_G; break;
            case 'M': g = ch_M; break;
            case 'O': g = ch_O; break;
            case 'P': g = ch_P; break;
            case 'R': g = ch_R; break;
            case 'S': g = ch_S; break;
            case 'T': g = ch_T; break;
            case 'V': g = ch_V; break;
            case ':': g = ch_colon; break;
            case '!': g = ch_bang; break;
            default: break;
        }
    }

    if (g) {
        for (int i = 0; i < 5; i++) out[i] = g[i];
    }
}

void ili9341_draw_text(int x, int y, const char *text, uint16_t color, int scale) {
    int cursor_x = x;
    while (*text) {
        uint8_t rows[5];
        get_glyph_3x5(*text, rows);

        for (int r = 0; r < 5; r++) {
            for (int c = 0; c < 3; c++) {
                if (rows[r] & (1 << (2 - c))) {
                    ili9341_fill_rect(cursor_x + c * scale, y + r * scale,
                                      scale, scale, color);
                }
            }
        }
        cursor_x += 4 * scale;
        text++;
    }
}

void ili9341_flush(void) {
    uint8_t col_range[4] = {0x00, 0x00, (SCREEN_W - 1) >> 8, (SCREEN_W - 1) & 0xFF};
    uint8_t row_range[4] = {0x00, 0x00, (SCREEN_H - 1) >> 8, (SCREEN_H - 1) & 0xFF};

    write_cmd(0x2A); /* CASET */
    write_data(col_range, 4);

    write_cmd(0x2B); /* PASET */
    write_data(row_range, 4);

    write_cmd(0x2C); /* RAMWR */
    gpio_put(ILI9341_PIN_DC, 1);
    gpio_put(ILI9341_PIN_CS, 0);
    spi_write_blocking(ILI9341_SPI_PORT, (const uint8_t *)framebuffer, sizeof(framebuffer));
    gpio_put(ILI9341_PIN_CS, 1);
}