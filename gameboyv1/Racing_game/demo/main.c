#include <stdlib.h>
#include <ncurses.h>
#include <time.h>
#include "racing_game.h"

int main(void) {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    srand(time(NULL));
    spawn_obstacles();

    while (!isGameOver) {
        timeout(game_speed);
        fill_board();
        input();
        check_collision();
        update_obstacles();
        check_collision();
        draw_car();
        draw_obstacles();
        print_board();

        if (isGameOver) {
            game_over_screen();
        }
    }

    endwin();
    return 0;
}