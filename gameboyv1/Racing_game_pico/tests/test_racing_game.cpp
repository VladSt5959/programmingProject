#include <gtest/gtest.h>

extern "C" {
#include "racing_game.h"
}

class RacingGameTest : public ::testing::Test {
protected:
    void SetUp() override {
        high_score = 0;
        reset_game();
    }
};

TEST_F(RacingGameTest, MoveCarClampsAtTrackBounds) {
    int start_y = car_y;
    move_car(-CAR_STEP);
    EXPECT_EQ(car_y, start_y - CAR_STEP);

    move_car(-1000);
    EXPECT_EQ(car_y, TRACK_TOP);

    move_car(1000);
    EXPECT_EQ(car_y, TRACK_BOTTOM - CAR_HEIGHT);
}

TEST_F(RacingGameTest, SpawnObstaclesSetsValidBoundsAndSpacing) {
    EXPECT_EQ(obst_x[0], SCREEN_W);

    for (int i = 0; i < OBST_COUNT; i++) {
        EXPECT_GE(obst_y[i], TRACK_TOP);
        EXPECT_LE(obst_y[i], TRACK_BOTTOM - CAR_HEIGHT);
        EXPECT_EQ(scored[i], 0);
        if (i > 0) {
            EXPECT_GE(obst_x[i] - obst_x[i - 1], 10);
            EXPECT_LE(obst_x[i] - obst_x[i - 1], 33);
        }
    }
}

TEST_F(RacingGameTest, CheckCollisionDetectsOverlap) {
    for (int i = 0; i < OBST_COUNT; i++) {
        obst_x[i] = SCREEN_W;
        obst_y[i] = TRACK_TOP;
    }
    obst_x[0] = car_x + CAR_WIDTH - 1;
    obst_y[0] = car_y + CAR_HEIGHT - 1;

    check_collision();
    EXPECT_EQ(isGameOver, 1);
}

TEST_F(RacingGameTest, CheckCollisionIgnoresTouchingEdges) {
    for (int i = 0; i < OBST_COUNT; i++) {
        obst_x[i] = SCREEN_W;
        obst_y[i] = TRACK_TOP;
    }
    obst_x[0] = car_x + CAR_WIDTH;
    obst_y[0] = car_y;
    check_collision();
    EXPECT_EQ(isGameOver, 0);

    obst_x[0] = car_x;
    obst_y[0] = car_y + CAR_HEIGHT;
    check_collision();
    EXPECT_EQ(isGameOver, 0);
}

TEST_F(RacingGameTest, UpdateObstaclesScoresOnceAndTracksHighScore) {
    for (int i = 0; i < OBST_COUNT; i++) {
        obst_x[i] = 250;
        scored[i] = 0;
    }

    obst_x[0] = car_x - CAR_WIDTH + 2;
    update_obstacles();

    EXPECT_EQ(scored[0], 1);
    EXPECT_EQ(score, 10);
    EXPECT_EQ(high_score, 10);

    update_obstacles();
    EXPECT_EQ(score, 10);
}

TEST_F(RacingGameTest, UpdateObstaclesRespawnsOffScreenObstacle) {
    for (int i = 0; i < OBST_COUNT; i++) {
        obst_x[i] = SCREEN_W;
        scored[i] = 0;
    }

    obst_x[0] = -CAR_WIDTH + 2;
    scored[0] = 1;
    update_obstacles();

    EXPECT_GT(obst_x[0], SCREEN_W);
    EXPECT_GE(obst_y[0], TRACK_TOP);
    EXPECT_LE(obst_y[0], TRACK_BOTTOM - CAR_HEIGHT);
    EXPECT_EQ(scored[0], 0);
}

TEST_F(RacingGameTest, ResetGameRestoresStateAndPreservesHighScore) {
    score = 200;
    high_score = 200;
    game_speed = 20;
    car_y = TRACK_TOP;
    isGameOver = 1;

    reset_game();

    EXPECT_EQ(score, 0);
    EXPECT_EQ(high_score, 200);
    EXPECT_EQ(game_speed, 45);
    EXPECT_EQ(isGameOver, 0);
}