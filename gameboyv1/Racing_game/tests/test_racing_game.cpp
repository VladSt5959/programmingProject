#include <gtest/gtest.h>

extern "C" {
#include "racing_game.h"
}

class RacingGameTest : public ::testing::Test {
protected:
    void SetUp() override {
        high_score = 0;
        reset_game();
        fill_board();
    }
};

TEST_F(RacingGameTest, FillBoardDrawsBordersAndSpaces) {
    // Corners and edges should be '#'
    EXPECT_EQ(board[0 * col + 0], '#');
    EXPECT_EQ(board[0 * col + (col - 1)], '#');
    EXPECT_EQ(board[(row - 1) * col + 0], '#');
    EXPECT_EQ(board[(row - 1) * col + (col - 1)], '#');

    // Interior cells should be ' '
    EXPECT_EQ(board[1 * col + 1], ' ');
    EXPECT_EQ(board[(row / 2) * col + (col / 2)], ' ');
    EXPECT_EQ(board[(row - 2) * col + (col - 2)], ' ');
}

TEST_F(RacingGameTest, DrawCarPlacesCorrectRectangle) {
    draw_car();

    // Every pixel inside the car's length x width box should be 'H'
    for (int dx = 0; dx < length; dx++) {
        for (int dy = 0; dy < width; dy++) {
            EXPECT_EQ(board[(car_y + dy) * col + (car_x + dx)], 'H');
        }
    }

    // Pixels immediately outside the car should still be empty
    EXPECT_EQ(board[car_y * col + (car_x - 1)], ' ');
    EXPECT_EQ(board[car_y * col + (car_x + length)], ' ');
    EXPECT_EQ(board[(car_y - 1) * col + car_x], ' ');
    EXPECT_EQ(board[(car_y + width) * col + car_x], ' ');
}

TEST_F(RacingGameTest, MoveCarClampsAtWalls) {
    int start_y = car_y;
    move_car(-5);
    EXPECT_EQ(car_y, start_y - 5);

    move_car(5);
    EXPECT_EQ(car_y, start_y);

    // Pushing far past top wall should clamp to 1
    move_car(-1000);
    EXPECT_EQ(car_y, 1);

    // Pushing far past bottom wall should clamp to row - 1 - width
    move_car(1000);
    EXPECT_EQ(car_y, row - 1 - width);
}

TEST_F(RacingGameTest, SpawnObstaclesSetsValidBoundsAndOrder) {
    EXPECT_EQ(obst_x[0], col - 2);

    for (int i = 0; i < obst_count; i++) {
        EXPECT_GE(obst_y[i], 1);
        EXPECT_LE(obst_y[i], row - 1 - width);
        EXPECT_EQ(scored[i], 0);
        if (i > 0) {
            EXPECT_GE(obst_x[i] - obst_x[i - 1], 5);
            EXPECT_LE(obst_x[i] - obst_x[i - 1], 20);
        }
    }
}

TEST_F(RacingGameTest, CheckCollisionDetectsOverlap) {
    // Move all obstacles away first
    for (int i = 0; i < obst_count; i++) {
        obst_x[i] = col - 2;
        obst_y[i] = 1;
    }

    // Place obstacle 0 overlapping the car's bottom-right corner
    obst_x[0] = car_x + length - 1;
    obst_y[0] = car_y + width - 1;

    check_collision();
    EXPECT_EQ(isGameOver, 1);
}

TEST_F(RacingGameTest, CheckCollisionIgnoresNonOverlapping) {
    for (int i = 0; i < obst_count; i++) {
        obst_x[i] = col - 2;
        obst_y[i] = 1;
    }

    // Touching right edge (not overlapping)
    obst_x[0] = car_x + length;
    obst_y[0] = car_y;
    check_collision();
    EXPECT_EQ(isGameOver, 0);

    // Touching bottom edge (not overlapping)
    obst_x[0] = car_x;
    obst_y[0] = car_y + width;
    check_collision();
    EXPECT_EQ(isGameOver, 0);
}

TEST_F(RacingGameTest, UpdateObstaclesMovesAndScoresOnce) {
    for (int i = 0; i < obst_count; i++) {
        obst_x[i] = 200;
        scored[i] = 0;
    }

    // Position obstacle 0 so one step of -4 moves its tail past car_x
    obst_x[0] = car_x - length + 2;
    update_obstacles();

    EXPECT_EQ(obst_x[0], car_x - length - 2);
    EXPECT_EQ(scored[0], 1);
    EXPECT_EQ(score, 10);

    // Updating again should not double-score the same obstacle
    update_obstacles();
    EXPECT_EQ(score, 10);
}

TEST_F(RacingGameTest, UpdateObstaclesTriggersSpeedMilestone) {
    for (int i = 0; i < obst_count; i++) {
        obst_x[i] = 200;
        scored[i] = 0;
    }

    score = 140;
    next_milestone = 150;
    milestone_step = 150;
    game_speed = 60;

    obst_x[0] = car_x - length + 2;
    update_obstacles();

    EXPECT_EQ(score, 150);
    EXPECT_EQ(game_speed, 50);
    EXPECT_EQ(milestone_step, 225);
    EXPECT_EQ(next_milestone, 375);
}

TEST_F(RacingGameTest, UpdateObstaclesRespawnsOffScreenObstacle) {
    for (int i = 0; i < obst_count; i++) {
        obst_x[i] = col - 2;
        scored[i] = 0;
    }

    // Position obstacle 0 so moving left by 4 puts its tail <= 0
    obst_x[0] = -length + 3;
    scored[0] = 1;

    update_obstacles();

    EXPECT_GT(obst_x[0], col - 2);
    EXPECT_GE(obst_y[0], 1);
    EXPECT_LE(obst_y[0], row - 1 - width);
    EXPECT_EQ(scored[0], 0);
}

TEST_F(RacingGameTest, ResetGameRestoresStateAndKeepsHighScore) {
    score = 320;
    high_score = 320;
    game_speed = 20;
    car_y = 5;
    isGameOver = 1;

    reset_game();

    EXPECT_EQ(score, 0);
    EXPECT_EQ(high_score, 320);
    EXPECT_EQ(next_milestone, 150);
    EXPECT_EQ(milestone_step, 150);
    EXPECT_EQ(game_speed, 60);
    EXPECT_EQ(car_x, 30);
    EXPECT_EQ(car_y, row / 2);
    EXPECT_EQ(isGameOver, 0);
}