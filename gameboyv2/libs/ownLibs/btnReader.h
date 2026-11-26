#ifndef BTN_READER_H
#define BTN_READER_H

#include <stdbool.h>

// GPIO numbers. Buttons connect to GND; internal pull-ups are enabled.
#define BTN_RELOAD 0
#define BTN_LEFT   6
#define BTN_RIGHT  7
#define BTN_UP     3
#define BTN_DOWN   4
#define BTN_A      5
#define BTN_B      2
#define BTN_MENU   5
#define BTN_MENU2  2

#define PHOTORESISTOR_PIN 27

void initBtns(void);
// Poll once per main-loop iteration, before reading button states.
// Returns the legacy direction code: up=0, down=1, left=2, right=3.
int readBtns(void);
// One event per debounced press, valid until the next readBtns() call.
bool btnPressed(unsigned int pin);
// Debounced held state, for continuous game movement.
bool btnDown(unsigned int pin);

void initPhotoresistor(void);
// Call each main-loop iteration; prints ADC data at most once per second.
void printPhotoresistor(void);

#endif
