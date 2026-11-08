#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <time.h>

#define row 13
#define col 50

char board [row * col];
int car_x = 11;
int car_y = 6;
int obst_x;
int obst_y;
int isGameOver = 0;

void draw_car () {
    board [car_y * col + car_x] = '-';
}

void spawn_obstacles () {
    obst_x = col - 2;
    obst_y = rand() % (row - 2) + 1;
}

void update_obstacles () {
    obst_x--;
    if (obst_x <= 0) {
        spawn_obstacles ();
    }
}

void draw_obstacles () {
    board [obst_y * col + obst_x] = 'X';
}

void check_collision () {
    if (car_x == obst_x && car_y == obst_y) {
        isGameOver = 1;
    }
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
                isGameOver = 1;
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

    srand(time(NULL));
    spawn_obstacles ();

    while (!isGameOver) {
        fill_board ();
        input ();
        update_obstacles ();
        check_collision ();
        draw_car ();
        draw_obstacles ();
        print_board ();
    };

    endwin(); 
    return 0;
}