#ifndef GAME_2048_H
#define GAME_2048_H

// Start a new 2048 game, reset the 4x4 board and current score
// (keeping the best score), and draw the initial screen.
void game2048Init(void);

// Call once per main-loop iteration after readBtns().
// Handles directional moves, restart on game over/win, and exit to menu.
// Returns 1 when the player asks to return to the menu, otherwise 0.
int game2048Read(void);

// Call once per main-loop iteration to advance game state and
// redraw the HUD, 4x4 grid, and game over/win overlays.
void game2048Update(void);

#endif