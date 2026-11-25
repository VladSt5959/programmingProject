#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <time.h>

#define row 240
#define col 320
#define width 5
#define length 15
#define obst_count 25

int scored[obst_count];
int score = 0;
int high_score = 0;
int next_milestone = 150;
int milestone_step = 150;
int game_speed = 60;

char board [row * col];
int car_x = 30;
int car_y = row/2;
int obst_x[obst_count];
int obst_y[obst_count];
int isGameOver = 0;

void draw_car () {
    for (int dx = 0; dx < length; dx++) {
        for (int dy = 0; dy < width; dy++) {
            board [(car_y + dy) * col + (car_x + dx)] = 'H';
        }
    }
}

int get_safe_y (int current_index, int limit) {
    int new_y;
    int overlap;
    int attempts = 0;

    do {
        new_y = rand() % (row - 2 - (width - 1)) + 1;
        overlap = 0;

        for (int j = 0; j < limit; j++) {
            if (j == current_index) {
                continue;
            }
            if (abs(obst_x[current_index] - obst_x[j]) < length + 2 &&
                abs(new_y - obst_y[j]) < width + 1) {
                overlap = 1;
                break;
            }
        }
        attempts++;
    } while (overlap == 1 && attempts < 20);

    return new_y;
} 

void spawn_obstacles () {
    for (int i = 0; i <= obst_count - 1; i++) {
        if (i == 0) {
            obst_x[i] = col - 2;
        }
        else {
            obst_x[i] = obst_x[i - 1] + (rand() % 16 + 5);
        }
        obst_y[i]= get_safe_y(i, i);
        scored[i] = 0;
    }
}

void update_obstacles () {
    for (int i = 0; i <= obst_count - 1; i++) {
        obst_x[i] -= 4;
        if (!scored[i] && obst_x[i] + length - 1 < car_x) {
            scored[i] = 1;
            score += 10;
            if (score >= next_milestone) {
                if (game_speed > 10) {
                    game_speed -= 10;
                }
                milestone_step += 75;
                next_milestone += milestone_step;
            }
        }
        if (obst_x[i] + length - 1 <= 0) {
            int max_x = col - 2;
            for (int j = 0; j <= obst_count - 1; j++) {
                if (obst_x[j] > max_x) {
                    max_x = obst_x[j];
                }
            }
            obst_x[i] = max_x + (rand() % 16 + 5);
            obst_y[i] = get_safe_y(i, obst_count);
            scored[i] = 0;
        }
    }
}

void draw_obstacles () {
    for (int i = 0; i <= obst_count - 1; i++) {
        for (int dx = 0; dx < length; dx++) {
            for (int dy = 0; dy < width; dy++) {
                int px = obst_x[i] + dx;
                int py = obst_y[i] + dy;
                if (px > 0 && py > 0 && px < col - 1 && py < row - 1) {
                    board [py * col + px] = 'X';
                }
            }
        }
    }
}

void check_collision () {
    for (int i = 0; i <= obst_count - 1; i++) {
        if (car_x < obst_x[i] + length && car_x + length > obst_x[i]) {
            if (car_y < obst_y[i] + width && car_y + width > obst_y[i]) {
                isGameOver = 1;
            }
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
    if (car_y < 1) {
        car_y = 1;
    }
    if (car_y + width - 1 > row - 2) {
        car_y = row - 1 - width;
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
                move_car (-5);
                break;
            case 's':
            case 'S':
            case KEY_DOWN:
                move_car (5);
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
    game_speed = 60;
    car_x = 30;
    car_y = row/2;
    isGameOver = 0;

    spawn_obstacles();
}

void game_over_screen() {
    if (score > high_score) {
        high_score = score;
    }
    erase();
    mvprintw((row / 2) - 3, (col / 2) - 5, "GAME OVER!");
    mvprintw((row / 2) - 1, (col / 2) - 8, "Final Score: %d", score);
    mvprintw((row / 2) + 1, (col / 2) - 9, "Highest Score: %d", high_score);
    mvprintw((row / 2) + 3, (col / 2) - 15, "Press R to Restart or Q to Quit");
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