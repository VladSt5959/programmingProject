#include <stdio.h>
#include <stdbool.h>
#include <ncurses.h>

#define row 13
#define col 50

char board [row * col];
int car_x = 11;
int car_y = 6;
int isGameOver = 0;

void draw_car () {
    board [car_y * col + car_x] = '.';
}

void fill_board() {
    int x, y;
    for (y = 0; y < row; y++) {
        for (x = 0; x < col; x++) {
            if (x == 0 || y == 0 || y == row - 1 || x == col - 1) {
                board [y * col + x] = '#';
            } 
            else {
                board [y * col + x] = ' ';
            }
        }
    }
}

void print_board() {
    erase();
    int x, y;
    for (y = 0; y < row; y++) {
        for (x = 0; x < col; x++) {
                mvaddch(y, x, board[y * col + x]);
        }
    }
    refresh();
}


void move_car(int diffY) {
    car_y += diffY;
    if (car_y == 0 || car_y == row - 1) {
        car_y -= diffY;
    }
}

void input() {
    int mv = getch();
    if (mv == ERR) {
        return;
    }
    else {
        switch (mv) {
            case 'w':
            case 'W':
            case KEY_UP:
                move_car (-1);
                break;
            case 's':
            case 'S':
            case KEY_DOWN:
                move_car (1);
                break;
            case 'q':
            case 'Q':
                isGameOver = 0;
                break;
        }
    }
}

int main () {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    timeout(100);

    while (!isGameOver) {
        fill_board ();
        draw_car ();
        print_board ();
        input ();
    };

    endwin(); 
    return 0;
}