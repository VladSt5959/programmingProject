#include <stdlib.h>
#include "racing_game.h"

int scored[OBST_COUNT];
int score = 0;
int high_score = 0;
int next_milestone = 150;
int milestone_step = 150;
int game_speed = 45;

int car_x = 30;
int car_y = TRACK_TOP + (TRACK_BOTTOM - TRACK_TOP - CAR_HEIGHT) / 2;
int obst_x[OBST_COUNT];
int obst_y[OBST_COUNT];
int isGameOver = 0;

int get_safe_y(int current_index, int limit) {
    int new_y;
    int overlap;
    int attempts = 0;
    int usable_height = TRACK_BOTTOM - TRACK_TOP - CAR_HEIGHT + 1;

    do {
        new_y = TRACK_TOP + (rand() % usable_height);
        overlap = 0;

        for (int j = 0; j < limit; j++) {
            if (j == current_index) {
                continue;
            }
            if (abs(obst_x[current_index] - obst_x[j]) < CAR_WIDTH + 8 &&
                abs(new_y - obst_y[j]) < CAR_HEIGHT + 4) {
                overlap = 1;
                break;
            }
        }
        attempts++;
    } while (overlap == 1 && attempts < 20);

    return new_y;
}

void spawn_obstacles(void) {
    for (int i = 0; i < OBST_COUNT; i++) {
        if (i == 0) {
            obst_x[i] = SCREEN_W;
        } else {
            obst_x[i] = obst_x[i - 1] + (rand() % 24 + 10);
        }
        obst_y[i] = get_safe_y(i, i);
        scored[i] = 0;
    }
}

void update_obstacles(void) {
    for (int i = 0; i < OBST_COUNT; i++) {
        obst_x[i] -= OBST_STEP;

        if (!scored[i] && obst_x[i] + CAR_WIDTH - 1 < car_x) {
            scored[i] = 1;
            score += 10;
            if (score > high_score) {
                high_score = score;
            }
            if (score >= next_milestone) {
                if (game_speed > 16) {
                    game_speed -= 5;
                }
                milestone_step += 75;
                next_milestone += milestone_step;
            }
        }

        if (obst_x[i] + CAR_WIDTH <= 0) {
            int max_x = SCREEN_W;
            for (int j = 0; j < OBST_COUNT; j++) {
                if (obst_x[j] > max_x) {
                    max_x = obst_x[j];
                }
            }
            obst_x[i] = max_x + (rand() % 24 + 10);
            obst_y[i] = get_safe_y(i, OBST_COUNT);
            scored[i] = 0;
        }
    }
}

void check_collision(void) {
    for (int i = 0; i < OBST_COUNT; i++) {
        if (car_x < obst_x[i] + CAR_WIDTH && car_x + CAR_WIDTH > obst_x[i]) {
            if (car_y < obst_y[i] + CAR_HEIGHT && car_y + CAR_HEIGHT > obst_y[i]) {
                isGameOver = 1;
            }
        }
    }
}

void move_car(int diffY) {
    car_y += diffY;
    if (car_y < TRACK_TOP) {
        car_y = TRACK_TOP;
    }
    if (car_y > TRACK_BOTTOM - CAR_HEIGHT) {
        car_y = TRACK_BOTTOM - CAR_HEIGHT;
    }
}

void reset_game(void) {
    score = 0;
    next_milestone = 150;
    milestone_step = 150;
    game_speed = 45;
    car_x = 30;
    car_y = TRACK_TOP + (TRACK_BOTTOM - TRACK_TOP - CAR_HEIGHT) / 2;
    isGameOver = 0;

    spawn_obstacles();
}