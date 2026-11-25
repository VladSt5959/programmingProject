#include "hardware/gpio.h"
#include "buttons.h"

static void init_btn(uint pin) {
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);
    gpio_pull_up(pin);
}

void buttons_init(void) {
    init_btn(BTN_UP_PIN);
    init_btn(BTN_DOWN_PIN);
    init_btn(BTN_RESTART_PIN);
}

bool button_up_pressed(void) {
    return !gpio_get(BTN_UP_PIN);
}

bool button_down_pressed(void) {
    return !gpio_get(BTN_DOWN_PIN);
}

bool button_restart_pressed(void) {
    return !gpio_get(BTN_RESTART_PIN);
}