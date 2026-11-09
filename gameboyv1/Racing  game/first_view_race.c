#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <time.h>

#define row 9
#define col 30
#define obst_count 10

int score = 0;
int high_score = 0;
int next_milestone = 150;
int milestone_step = 150;
int game_speed = 88;

char board [row * col];
int car_x = 6;
int car_y = 4;
int obst_x[obst_count];
int obst_y[obst_count];
int isGameOver = 0;

void draw_car () {
    board [car_y * col + car_x] = 'H';
}

void spawn_obstacles () {
    for (int i = 0; i <= obst_count - 1; i++) {
        if (i == 0) {
            obst_x[i] = col - 2;
        }
        else {
            obst_x[i] = obst_x[i - 1] + (rand() % 5 + 1);
        }
        obst_y[i] = rand() % (row - 2) + 1;
    }
}

void update_obstacles () {
    for (int i = 0; i <= obst_count - 1; i++) {
        obst_x[i] -= 1;
        if (obst_x[i] == car_x - 1) {
            score += 10;
            if (score >= next_milestone && game_speed > 20) {
                game_speed -= 10;
                milestone_step += 150;
                next_milestone += milestone_step;
            }
        }
        if (obst_x[i] <= 0) {
            int max_x = col - 2;
            for (int j = 0; j <= obst_count - 1; j++) {
                if (obst_x[j] > max_x) {
                    max_x = obst_x[j];
                }
            }
            obst_x[i] = max_x + (rand() % 5 + 1);
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
    mvprintw(row, 0, "Score: %d | Next speed up: %d", score, next_milestone);
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

void reset_game () {
    score = 0;
    next_milestone = 150;
    milestone_step = 150;
    game_speed = 88;
    car_x = 6;
    car_y = 4;
    isGameOver = 0;

    spawn_obstacles();
}

void game_over_screen() {
    if (score > high_score) {
        high_score = score;
    }
    erase();
    mvprintw(2, (col / 2) - 5, "GAME OVER!");
    mvprintw(4, (col / 2) - 8, "Final Score: %d", score);
    mvprintw(6, (col / 2) - 9, "Highest Score: %d", high_score);
    mvprintw(8, (col / 2) - 15, "Press R to Restart or Q to Quit");
    refresh();

    timeout(-1);

    while (isGameOver) {
        int r = getch();
        switch (r) {
            case 'r':
            case 'R':
                reset_game();
                break;
            case 'q':
            case 'Q':
                return;
        }
    }
}

int main () {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    srand(time(NULL));
    spawn_obstacles ();

    while (!isGameOver) {
        timeout(game_speed);
        fill_board ();
        input ();
        check_collision ();
        update_obstacles ();
        check_collision ();
        draw_car ();
        draw_obstacles ();
        print_board ();

        if (isGameOver) {
            game_over_screen();
        }
    };

    endwin(); 
    return 0;
}