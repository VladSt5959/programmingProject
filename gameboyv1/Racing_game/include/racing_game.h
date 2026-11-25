#ifndef RACING_GAME_H
#define RACING_GAME_H

#define row 240
#define col 320
#define width 5
#define length 15
#define obst_count 25

extern int scored[obst_count];
extern int score;
extern int high_score;
extern int next_milestone;
extern int milestone_step;
extern int game_speed;

extern char board[row * col];
extern int car_x;
extern int car_y;
extern int obst_x[obst_count];
extern int obst_y[obst_count];
extern int isGameOver;

void draw_car(void);
int get_safe_y(int current_index, int limit);
void spawn_obstacles(void);
void update_obstacles(void);
void draw_obstacles(void);
void check_collision(void);
void fill_board(void);
void print_board(void);
void move_car(int diffY);
void input(void);
void reset_game(void);
void game_over_screen(void);

#endif