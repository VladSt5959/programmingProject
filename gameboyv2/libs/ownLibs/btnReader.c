#include "btnReader.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "pico/bootrom.h"
#include <stdio.h>

#define DEBOUNCE_MS 20

typedef struct {
    unsigned int pin;
    bool raw;
    bool down;
    bool pressed;
    uint32_t changed_at;
} Button;

static Button buttons[] = {
    {.pin = BTN_LEFT}, {.pin = BTN_RIGHT},
    {.pin = BTN_UP}, {.pin = BTN_DOWN},
    {.pin = BTN_A}, {.pin = BTN_B},
    {.pin = BTN_MENU}, {.pin = BTN_MENU2}, {.pin = BTN_RELOAD}
};

#define BUTTON_COUNT (sizeof(buttons) / sizeof(buttons[0]))

void initBtns(void) {
    for (size_t i = 0; i < BUTTON_COUNT; i++) {
        gpio_init(buttons[i].pin);
        gpio_set_dir(buttons[i].pin, GPIO_IN);
        gpio_pull_up(buttons[i].pin);
    }
    sleep_ms(1);
    uint32_t now = to_ms_since_boot(get_absolute_time());
    for (size_t i = 0; i < BUTTON_COUNT; i++) {
        Button *btn = &buttons[i];
        btn->raw = !gpio_get(btn->pin);
        btn->down = btn->raw;
        btn->pressed = false;
        btn->changed_at = now;
    }
}

bool btnPressed(unsigned int pin) {
    for (size_t i = 0; i < BUTTON_COUNT; i++) {
        if (buttons[i].pin == pin) return buttons[i].pressed;
    }
    return false;
}

bool btnDown(unsigned int pin) {
    for (size_t i = 0; i < BUTTON_COUNT; i++) {
        if (buttons[i].pin == pin) return buttons[i].down;
    }
    return false;
}

int readBtns(void) {
    uint32_t now = to_ms_since_boot(get_absolute_time());
    for (size_t i = 0; i < BUTTON_COUNT; i++) {
        Button *btn = &buttons[i];
        bool raw = !gpio_get(btn->pin);
        btn->pressed = false;

        if (raw != btn->raw) {
            btn->raw = raw;
            btn->changed_at = now;
        }
        if (raw != btn->down &&
            (uint32_t)(now - btn->changed_at) >= DEBOUNCE_MS) {
            btn->down = raw;
            btn->pressed = raw;
            if (btn->pressed) printf("btn %u clicked\n", btn->pin);
        }
    }

    if (btnPressed(BTN_RELOAD)) {
        stdio_flush();
        reset_usb_boot(0, 0);
    }
    if (btnDown(BTN_UP)) return 0;
    if (btnDown(BTN_DOWN)) return 1;
    if (btnDown(BTN_LEFT)) return 2;
    if (btnDown(BTN_RIGHT)) return 3;
    return -1;
}

// GP26 is ADC input 0 on Pico / Pico 2.
static uint32_t photoresistorLastPrint;

void initPhotoresistor(void) {
    adc_init();
    adc_gpio_init(PHOTORESISTOR_PIN);
    photoresistorLastPrint = to_ms_since_boot(get_absolute_time());
}

void printPhotoresistor(void) {
    uint32_t now = to_ms_since_boot(get_absolute_time());
    if ((uint32_t)(now - photoresistorLastPrint) < 1000) return;
    photoresistorLastPrint = now;

    adc_select_input(PHOTORESISTOR_PIN - 26);
    unsigned int raw = adc_read();
    printf("photoresistor GP%u: %u\n", (unsigned int)PHOTORESISTOR_PIN, raw);
}

