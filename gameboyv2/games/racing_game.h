#ifndef RACINGGAME_TEMP_H
#define RACINGGAME_TEMP_H

// Start a new race and draw its initial screen.
void racingGameInit(void);

// Call once per main-loop iteration after readBtns().
// Returns 1 when the player asks to return to the menu.
int racingGameRead(void);

// Call once per main-loop iteration to advance and draw the race.
void racingGameUpdate(void);

#endif
