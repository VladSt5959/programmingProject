#ifndef BTN_READER_H
#define BTN_READER_H

#include <stdbool.h>

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
int readBtns(void);
bool btnPressed(unsigned int pin);
bool btnDown(unsigned int pin);

void initPhotoresistor(void);
void printPhotoresistor(void);

#endif
