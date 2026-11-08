#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <time.h>

#define row 9
#define col 30
#define obst_count 7

char board [row * col];
int car_x = 6;
int car_y = 4;
int obst_x[obst_count];
int obst_y[obst_count];
int isGameOver = 0;

void draw_car () {
    board [car_y * col + car_x] = '-';
}

void spawn_obstacles () {
    for (int i = 0; i <= obst_count - 1; i++) {
        obst_x[i] = (col - 2) + i * (col / obst_count);
        obst_y[i] = rand() % (row - 2) + 1;
    }
}

void update_obstacles () {
    for (int i = 0; i <= obst_count - 1; i++) {
        obst_x[i] -= 1;
        if (obst_x[i] <= 0) {
            obst_x[i] = col - 2;
            obst_y[i] = rand() % (row - 2) + 1;
        }
    }
}

void draw_obstacles () {
    for (int i = 0; i <= obst_count - 1; i++) {
        if (obst_x[i] > 0 && obst_x[i] < col - 1) {
            board [obst_y[i] * col + obst_x[i]] = 'X';
        }
    }
}

void check_collision () {
    for (int i = 0; i <= obst_count - 1; i++) {
        if (car_x == obst_x[i] && car_y == obst_y[i]) {
        isGameOver = 1;
        }
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
    timeout(50);

    srand(time(NULL));
    spawn_obstacles ();

    while (!isGameOver) {
        fill_board ();
        input ();
        check_collision ();
        update_obstacles ();
        check_collision ();
        draw_car ();
        draw_obstacles ();
        print_board ();
    };

    endwin(); 
    return 0;
}