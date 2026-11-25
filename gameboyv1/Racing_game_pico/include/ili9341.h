#ifndef ILI9341_H
#define ILI9341_H

#include <stdint.h>
#include "hardware/spi.h"

#define ILI9341_SPI_PORT spi0
#define ILI9341_SPI_BAUD (40 * 1000 * 1000)

#define ILI9341_PIN_SCK  18
#define ILI9341_PIN_MOSI 19
#define ILI9341_PIN_CS   17
#define ILI9341_PIN_DC   16
#define ILI9341_PIN_RST  20
#define ILI9341_PIN_BL   21

static inline uint16_t ili9341_rgb565(uint8_t r, uint8_t g, uint8_t b) {
    uint16_t c = ((uint16_t)(r & 0xF8) << 8) |
                 ((uint16_t)(g & 0xFC) << 3) |
                 ((uint16_t)(b >> 3));
    return (uint16_t)((c >> 8) | (c << 8));
}

void ili9341_init(void);
void ili9341_clear(uint16_t color);
void ili9341_fill_rect(int x, int y, int w, int h, uint16_t color);
void ili9341_draw_text(int x, int y, const char *text, uint16_t color, int scale);
void ili9341_flush(void);

#endif