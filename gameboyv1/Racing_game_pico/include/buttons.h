#ifndef BUTTONS_H
#define BUTTONS_H

#include <stdbool.h>

#define BTN_UP_PIN      10
#define BTN_DOWN_PIN    11
#define BTN_RESTART_PIN 12

void buttons_init(void);
bool button_up_pressed(void);
bool button_down_pressed(void);
bool button_restart_pressed(void);

#endif