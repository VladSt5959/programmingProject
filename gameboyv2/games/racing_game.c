#include "racing_game.h"

#include "pico/stdlib.h"
#include "libs/ownLibs/btnReader.h"
#include "libs/gfx/gfx.h"
#include "colors.h"

#include <stdint.h>
#include <stdlib.h>

#define RACE_SCREEN_W       320
#define RACE_SCREEN_H       240
#define RACE_HUD_H          20
#define RACE_TRACK_TOP      (RACE_HUD_H + 3)
#define RACE_TRACK_BOTTOM   (RACE_SCREEN_H - 3)

#define RACE_CAR_W          26
#define RACE_CAR_H          16
#define RACE_CAR_X          28
#define RACE_CAR_STEP       5

#define RACE_OBSTACLE_COUNT 8
#define RACE_OBSTACLE_W     24
#define RACE_OBSTACLE_H     16
#define RACE_OBSTACLE_STEP  4
#define RACE_OBSTACLE_GAP   56

#define RACE_INITIAL_DELAY  45
#define RACE_MIN_DELAY      16

static int carY;
static int obstacleX[RACE_OBSTACLE_COUNT];
static int obstacleY[RACE_OBSTACLE_COUNT];
static bool obstacleScored[RACE_OBSTACLE_COUNT];

static int raceScore;
static int raceBestScore;
static int nextSpeedMilestone;
static int raceDelayMs;
static bool raceGameOver;
static uint32_t nextFrameAt;

static uint32_t raceNowMs(void)
{
    return to_ms_since_boot(get_absolute_time());
}

static int randomObstacleY(void)
{
    const int minY = RACE_TRACK_TOP + 4;
    const int maxY = RACE_TRACK_BOTTOM - RACE_OBSTACLE_H - 4;
    return minY + (rand() % (maxY - minY + 1));
}

static void drawCar(int x, int y, uint16_t color)
{
    GFX_fillRect(x, y + 3, RACE_CAR_W, RACE_CAR_H - 6, color);
    GFX_fillRect(x + 5, y, RACE_CAR_W - 10, RACE_CAR_H, color);
    GFX_fillRect(x + 8, y + 3, 8, 4, CYAN);
    GFX_fillRect(x + 4, y + RACE_CAR_H - 2, 5, 3, BLACK);
    GFX_fillRect(x + RACE_CAR_W - 9, y + RACE_CAR_H - 2, 5, 3, BLACK);
}

static void drawObstacle(int x, int y)
{
    GFX_fillRect(x, y + 3, RACE_OBSTACLE_W, RACE_OBSTACLE_H - 6, RED);
    GFX_fillRect(x + 4, y, RACE_OBSTACLE_W - 8, RACE_OBSTACLE_H, RED);
    GFX_fillRect(x + 6, y + 3, 5, 4, YELLOW);
    GFX_fillRect(x + RACE_OBSTACLE_W - 11, y + 3, 5, 4, YELLOW);
}

static void renderRace(void)
{
    GFX_fillScreen(DARKGRAY);

    // HUD
    GFX_fillRect(0, 0, RACE_SCREEN_W, RACE_HUD_H, BLACK);
    GFX_setTextColor(WHITE);
    GFX_setTextBack(BLACK);
    GFX_setTextSize(1);
    GFX_setCursor(4, 5);
    GFX_printf("RACE  SCORE %d  BEST %d", raceScore, raceBestScore);

    // Track edges and dashed center line.
    GFX_fillRect(0, RACE_TRACK_TOP, RACE_SCREEN_W, 3, WHITE);
    GFX_fillRect(0, RACE_TRACK_BOTTOM - 3, RACE_SCREEN_W, 3, WHITE);
    for (int x = 0; x < RACE_SCREEN_W; x += 32) {
        GFX_fillRect(x, (RACE_TRACK_TOP + RACE_TRACK_BOTTOM) / 2,
                     16, 2, YELLOW);
    }

    for (int i = 0; i < RACE_OBSTACLE_COUNT; i++) {
        drawObstacle(obstacleX[i], obstacleY[i]);
    }
    drawCar(RACE_CAR_X, carY, GREEN);

    GFX_flush();
}

static void renderGameOver(void)
{
    renderRace();
    GFX_fillRect(55, 91, 210, 58, BLACK);
    GFX_drawRect(55, 91, 210, 58, RED);
    GFX_setTextColor(RED);
    GFX_setTextBack(BLACK);
    GFX_setTextSize(2);
    GFX_setCursor(104, 98);
    GFX_printf("CRASH!");
    GFX_setTextColor(WHITE);
    GFX_setTextSize(1);
    GFX_setCursor(75, 128);
    GFX_printf("A: RETRY   B: MENU");
    GFX_flush();
}

static void resetRace(void)
{
    raceScore = 0;
    nextSpeedMilestone = 100;
    raceDelayMs = RACE_INITIAL_DELAY;
    raceGameOver = false;
    carY = RACE_TRACK_TOP +
           (RACE_TRACK_BOTTOM - RACE_TRACK_TOP - RACE_CAR_H) / 2;

    for (int i = 0; i < RACE_OBSTACLE_COUNT; i++) {
        obstacleX[i] = RACE_SCREEN_W + i * RACE_OBSTACLE_GAP;
        obstacleY[i] = randomObstacleY();
        obstacleScored[i] = false;
    }

    nextFrameAt = raceNowMs();
}

static void moveCar(int dy)
{
    carY += dy;
    if (carY < RACE_TRACK_TOP + 3) {
        carY = RACE_TRACK_TOP + 3;
    }
    if (carY > RACE_TRACK_BOTTOM - 3 - RACE_CAR_H) {
        carY = RACE_TRACK_BOTTOM - 3 - RACE_CAR_H;
    }
}

static bool carsCollide(int x1, int y1, int w1, int h1,
                        int x2, int y2, int w2, int h2)
{
    return x1 < x2 + w2 && x1 + w1 > x2 &&
           y1 < y2 + h2 && y1 + h1 > y2;
}

static void updateRace(void)
{
    for (int i = 0; i < RACE_OBSTACLE_COUNT; i++) {
        obstacleX[i] -= RACE_OBSTACLE_STEP;

        if (!obstacleScored[i] &&
            obstacleX[i] + RACE_OBSTACLE_W < RACE_CAR_X) {
            obstacleScored[i] = true;
            raceScore += 10;
            if (raceScore > raceBestScore) {
                raceBestScore = raceScore;
            }

            if (raceScore >= nextSpeedMilestone) {
                if (raceDelayMs > RACE_MIN_DELAY) {
                    raceDelayMs -= 3;
                    if (raceDelayMs < RACE_MIN_DELAY) {
                        raceDelayMs = RACE_MIN_DELAY;
                    }
                }
                nextSpeedMilestone += 100;
            }
        }

        if (obstacleX[i] + RACE_OBSTACLE_W < 0) {
            int farthestX = RACE_SCREEN_W;
            for (int j = 0; j < RACE_OBSTACLE_COUNT; j++) {
                if (j != i && obstacleX[j] > farthestX) {
                    farthestX = obstacleX[j];
                }
            }
            obstacleX[i] = farthestX + RACE_OBSTACLE_GAP;
            obstacleY[i] = randomObstacleY();
            obstacleScored[i] = false;
        }

        if (carsCollide(RACE_CAR_X, carY, RACE_CAR_W, RACE_CAR_H,
                        obstacleX[i], obstacleY[i],
                        RACE_OBSTACLE_W, RACE_OBSTACLE_H)) {
            raceGameOver = true;
            return;
        }
    }
}

void racingGameInit(void)
{
    resetRace();
    renderRace();
}

int racingGameRead(void)
{
    if (btnPressed(BTN_B) ) {
        return 1;
    }

    if (raceGameOver) {
        if (btnPressed(BTN_A)) {
            resetRace();
            renderRace();
        }
        return 0;
    }

    return 0;
}

void racingGameUpdate(void)
{
    if (raceGameOver) {
        return;
    }

    uint32_t now = raceNowMs();
    if ((int32_t)(now - nextFrameAt) < 0) {
        return;
    }
    nextFrameAt = now + raceDelayMs;

    if (btnDown(BTN_UP)) {
        moveCar(-RACE_CAR_STEP);
    }
    if (btnDown(BTN_DOWN)) {
        moveCar(RACE_CAR_STEP);
    }

    updateRace();
    if (raceGameOver) {
        renderGameOver();
    } else {
        renderRace();
    }

}
