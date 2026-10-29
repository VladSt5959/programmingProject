#include <stdio.h>
#include <stdbool.h>

#define row 13
#define col 50

char board [row * col];
int car_x = 11;
int car_y = 6;
int GameOver = 0;

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
        }
    }
}

void print_board() {
    int x, y;
    int p = 1;
    for (y = 0; y < row; y++) {
        for (x = 0; x < col; x++) {
            if (y == car_y && x == 11) {
                printf ("%c", board [car_y * col + car_x]);
            }
            if (y > 0 && y < row - 1 && x > 0 && x < col - 1) {
                printf("%*s", p, "");
            }
            else {
            printf ("%c", board [y * col + x]);
            }
        }
        printf ("\n");
    }
}

void move_car() {

}

void input() {
    int dir = getchar();
    
    switch (dir) {
        case 'w':
            move_car (0, -1);
            break;
        case 's':
            move_car (0, 1);
            break;
    }
}

int main () {
    while (true) {
        fill_board ();
        draw_car ();
        print_board ();
        input ();
    };
    return 0;
}