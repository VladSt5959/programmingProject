#ifndef RACING_GAME_H
#define RACING_GAME_H

#define SCREEN_W         320
#define SCREEN_H         240
#define HUD_HEIGHT       20
#define BORDER_THICKNESS 2

#define TRACK_TOP        (HUD_HEIGHT + BORDER_THICKNESS)
#define TRACK_BOTTOM     (SCREEN_H - BORDER_THICKNESS)

#define CAR_WIDTH        24
#define CAR_HEIGHT       12
#define OBST_COUNT       16
#define OBST_STEP        4
#define CAR_STEP         5

extern int scored[OBST_COUNT];
extern int score;
extern int high_score;
extern int next_milestone;
extern int milestone_step;
extern int game_speed;

extern int car_x;
extern int car_y;
extern int obst_x[OBST_COUNT];
extern int obst_y[OBST_COUNT];
extern int isGameOver;

int get_safe_y(int current_index, int limit);
void spawn_obstacles(void);
void update_obstacles(void);
void check_collision(void);
void move_car(int diffY);
void reset_game(void);

#endif