#include "platformer.h"
#include "colors.h"
#include "gfx.h"
#include "pico/stdlib.h"
#include "libs/ownLibs/btnReader.h"
#include "colors.h"
#include <stdint.h>

#define SCREEN_W       320
#define SCREEN_H       240
#define HUD_H           20
#define GROUND_Y       208
#define PLAYER_W        16
#define PLAYER_H        16
#define ENEMY_SIZE      16
#define PLATFORM_MAX    12
#define ENEMY_MAX        5
#define COIN_MAX        10
#define LEVEL_COUNT      3
#define PLATFORMER_TRANSPARENT  0x0000

#define GRAVITY       0.48f
#define JUMP_SPEED   -8.2f
#define RUN_SPEED     2.35f
#define MAX_FALL      8.0f

enum { TILE_GROUND, TILE_BRICK, TILE_QUESTION, TILE_USED };

typedef struct {
    int x, y, w, h, type;
    uint8_t used;
} Platform;

typedef struct {
    float x, y, vx;
    int left, right;
    uint8_t active;
} Enemy;

typedef struct {
    int x, y;
    uint8_t active;
} Coin;

static Platform platforms[PLATFORM_MAX];
static Enemy enemies[ENEMY_MAX];
static Coin coinItems[COIN_MAX];
static int platformCount, enemyCount, coinCount;

static float playerX, playerY, playerVX, playerVY;
static int playerDir = 1;
static int score, coins, lives, currentLevel;
static int animationTick, hurtTicks;
static uint32_t rngState;
static bool grounded, gameOver, levelComplete, gameWon;
static int flagX;
static const uint16_t skyColors[LEVEL_COUNT] = { 0x867D, 0x4D7F, 0x1D3A };

static uint32_t randomU32(void)
{
    uint32_t x = rngState;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    rngState = x ? x : 0xA341316Cu;
    return rngState;
}

static int randomRange(int low, int high)
{
    return low + (int)(randomU32() % (uint32_t)(high - low + 1));
}

static void addPlatform(int x, int y, int w, int type)
{
    if (platformCount >= PLATFORM_MAX) return;
    Platform *p = &platforms[platformCount++];
    p->x = x; p->y = y; p->w = w; p->h = 16; p->type = type; p->used = 0;
}

static void addCoin(int x, int y)
{
    if (coinCount >= COIN_MAX) return;
    coinItems[coinCount++] = (Coin){ x, y, 1 };
}

static void addEnemy(int x, int y, int left, int right, float speed)
{
    if (enemyCount >= ENEMY_MAX) return;
    Enemy *e = &enemies[enemyCount++];
    e->x = (float)x; e->y = (float)y; e->vx = speed;
    e->left = left; e->right = right; e->active = 1;
}

static void generateLevel(int level)
{
    platformCount = enemyCount = coinCount = 0;
    addPlatform(0, GROUND_Y, SCREEN_W, TILE_GROUND);

    int x = randomRange(26, 34);
    int y = 174;
    const int steps = 6 + (level > 0);
    for (int i = 0; i < steps; ++i) {
        if (i > 0) {
            x += randomRange(33, 38);
            y += randomRange(-22, 22);
            if (y < 112) y = 112 + randomRange(0, 8);
            if (y > 178) y = 178 - randomRange(0, 8);
        }
        int width = (randomRange(0, 2) == 0) ? 32 : 48;
        int type = randomRange(0, 3) == 0 ? TILE_QUESTION : TILE_BRICK;
        addPlatform(x, y, width, type);

       if (i < COIN_MAX - 1) addCoin(x + randomRange(5, width - 12), y - randomRange(18, 30));
    }

    if (coinCount < COIN_MAX) addCoin(randomRange(72, 116), randomRange(168, 184));
    int count = 2 + level + randomRange(0, 1);
    for (int i = 0; i < count && i < ENEMY_MAX; ++i) {
        int left = 65 + i * 67;
        int right = left + randomRange(34, 54);
        if (right > SCREEN_W - ENEMY_SIZE) right = SCREEN_W - ENEMY_SIZE;
        addEnemy(randomRange(left, right), GROUND_Y - ENEMY_SIZE,
                 left, right, (randomRange(0, 1) ? 1.0f : -1.0f) * (0.7f + level * 0.18f));
    }
    flagX = 294;

    playerX = 18;
    playerY = GROUND_Y - PLAYER_H;
    playerVX = playerVY = 0;
    grounded = true;
    hurtTicks = 0;
}

void platformerInit(void)
{
    rngState = time_us_32() ^ 0x9E3779B9u;
    if (!rngState) rngState = 0xA341316Cu;
    score = coins = 0;
    lives = 3;
    currentLevel = 0;
    gameOver = levelComplete = gameWon = false;
    animationTick = 0;
    generateLevel(currentLevel);
}

int platformerRead(void)
{
    if (btnPressed(BTN_B) ) return 1;

    if (gameOver || gameWon) {
        if (btnPressed(BTN_A)) platformerInit();
        return 0;
    }
    if (levelComplete) {
        if (btnPressed(BTN_A)) {
            if (++currentLevel >= LEVEL_COUNT) gameWon = true;
            else {
                levelComplete = false;
                rngState ^= 0x85EBCA6Bu + (uint32_t)currentLevel;
                generateLevel(currentLevel);
            }
        }
        return 0;
    }

    playerVX = 0;
    if (btnDown(BTN_LEFT))  { playerVX = -RUN_SPEED; playerDir = -1; }
    if (btnDown(BTN_RIGHT)) { playerVX = RUN_SPEED; playerDir = 1; }
    if ((btnPressed(BTN_UP) || btnPressed(BTN_A)) && grounded) {
        playerVY = JUMP_SPEED;
        grounded = false;
    }
    return 0;
}

static bool overlaps(float x, float y, int w, int h, const Platform *p)
{
    return x + w > p->x && x < p->x + p->w && y + h > p->y && y < p->y + p->h;
}

static void hitPlayer(void)
{
    if (hurtTicks > 0) return;
    if (--lives <= 0) { gameOver = true; return; }
    playerX = 18; playerY = GROUND_Y - PLAYER_H;
    playerVX = playerVY = 0;
    grounded = true;
    hurtTicks = 90;
}

static void updatePlayer(void)
{
    float oldY = playerY;
    float nextX = playerX + playerVX;
    if (nextX < 0) nextX = 0;
    if (nextX > SCREEN_W - PLAYER_W) nextX = SCREEN_W - PLAYER_W;
    for (int i = 1; i < platformCount; ++i) {
        if (overlaps(nextX, playerY, PLAYER_W, PLAYER_H, &platforms[i])) {
            if (playerVX > 0) nextX = platforms[i].x - PLAYER_W;
            else if (playerVX < 0) nextX = platforms[i].x + platforms[i].w;
        }
    }
    playerX = nextX;

    playerVY += GRAVITY;
    if (playerVY > MAX_FALL) playerVY = MAX_FALL;
    float nextY = playerY + playerVY;
    grounded = false;
    for (int i = 0; i < platformCount; ++i) {
        Platform *p = &platforms[i];
        if (playerX + PLAYER_W <= p->x || playerX >= p->x + p->w) continue;
        if (playerVY >= 0 && oldY + PLAYER_H <= p->y && nextY + PLAYER_H >= p->y) {
            nextY = p->y - PLAYER_H;
            playerVY = 0;
            grounded = true;
        } else if (playerVY < 0 && oldY >= p->y + p->h && nextY <= p->y + p->h) {
            nextY = p->y + p->h;
            playerVY = 0;
            if (p->type == TILE_QUESTION && !p->used) {
                p->used = 1;
                p->type = TILE_USED;
                score += 100;
                if (coinCount < COIN_MAX) addCoin(p->x + p->w / 2 - 4, p->y - 12);
            }
        }
    }
    playerY = nextY;
    if (playerY > SCREEN_H) hitPlayer();
    if (hurtTicks > 0) --hurtTicks;
    ++animationTick;
}

static void updateEnemies(void)
{
    for (int i = 0; i < enemyCount; ++i) {
        Enemy *e = &enemies[i];
        if (!e->active) continue;
        e->x += e->vx;
        if (e->x < e->left) { e->x = (float)e->left; e->vx = -e->vx; }
        if (e->x > e->right) { e->x = (float)e->right; e->vx = -e->vx; }

        if (playerX + PLAYER_W > e->x && playerX < e->x + ENEMY_SIZE &&
            playerY + PLAYER_H > e->y && playerY < e->y + ENEMY_SIZE) {
            if (playerVY > 0 && playerY + PLAYER_H - e->y < 9) {
                e->active = 0;
                playerVY = -5.2f;
                grounded = false;
                score += 150;
            } else hitPlayer();
        }
    }
}

static void updateCoins(void)
{
    for (int i = 0; i < coinCount; ++i) {
        Coin *c = &coinItems[i];
        if (!c->active) continue;
        if (playerX + PLAYER_W > c->x && playerX < c->x + 8 &&
            playerY + PLAYER_H > c->y && playerY < c->y + 8) {
            c->active = 0;
            ++coins;
            score += 50;
            if (coins % 10 == 0) ++lives;
        }
    }
}

static void drawTransparent(int x, int y, const uint16_t *sprite, bool flip)
{
    for (int row = 0; row < 16; ++row) for (int col = 0; col < 16; ++col) {
        uint16_t color = sprite[row * 16 + (flip ? 15 - col : col)];
        if (color != PLATFORMER_TRANSPARENT) GFX_drawPixel(x + col, y + row, color);
    }
}

static void drawLevel(void)
{
    GFX_fillScreen(skyColors[currentLevel]);
   for (int i = 0; i < 5; ++i) {
        int cx = (i * 67 + 21) % SCREEN_W;
        int cy = 32 + (i * 17) % 40;
        GFX_fillRect(cx, cy, 18, 3, 0xBDF7);
        GFX_fillRect(cx + 4, cy - 3, 10, 3, 0xBDF7);
    }

    for (int i = 0; i < platformCount; ++i) {
        Platform *p = &platforms[i];
        const uint16_t *tile = p->type == TILE_GROUND ? sprite_ground :
                               p->type == TILE_QUESTION ? (p->used ? sprite_used : sprite_question) :
                               p->type == TILE_USED ? sprite_used : sprite_brick;
        for (int bx = p->x; bx < p->x + p->w; bx += 16)
            draw_image(bx, p->y, 16, 16, tile);
    }

    GFX_fillRect(flagX + 5, GROUND_Y - 78, 3, 78, 0xFFFF);
    draw_image(flagX + 8, GROUND_Y - 78, 16, 16, sprite_flag);

    for (int i = 0; i < coinCount; ++i)
        if (coinItems[i].active)
            drawTransparent(coinItems[i].x, coinItems[i].y + ((animationTick / 8) & 1),
                            (animationTick / 8 & 1) ? sprite_coin_b : sprite_coin_a, false);

    for (int i = 0; i < enemyCount; ++i) if (enemies[i].active)
        drawTransparent((int)enemies[i].x, (int)enemies[i].y,
                        (animationTick / 12 & 1) ? sprite_enemy_walk2 : sprite_enemy_walk1, false);
}

static void drawPlayer(void)
{
    if (hurtTicks > 0 && (hurtTicks / 5) % 2) return;
    const uint16_t *sprite = sprite_hero_idle;
    if (playerVX != 0 && (animationTick / 7) % 2) sprite = sprite_hero_run1;
    else if (playerVX != 0) sprite = sprite_hero_run2;
    drawTransparent((int)playerX, (int)playerY, sprite, playerDir < 0);
}

static void drawHUD(void)
{
    GFX_fillRect(0, 0, SCREEN_W, HUD_H, 0x0000);
    GFX_setTextColor(0xFFFF);
    GFX_setTextBack(0x0000);
    GFX_setTextSize(1);
    GFX_setCursor(5, 6);   GFX_printf("WORLD %d", currentLevel + 1);
    GFX_setCursor(88, 6);  GFX_printf("SCORE %d", score);
    GFX_setCursor(185, 6); GFX_printf("COIN %d", coins);
    GFX_setCursor(270, 6); GFX_printf("LIFE %d", lives);
}

static void drawMessage(const char *title, const char *line1, const char *line2, uint16_t color)
{
    GFX_fillRect(38, 76, 244, 92, 0x0000);
    GFX_drawRect(38, 76, 244, 92, color);
    GFX_setTextColor(color); GFX_setTextBack(0x0000); GFX_setTextSize(2);
    GFX_setCursor(68, 88); GFX_printf(title);
    GFX_setTextColor(0xFFFF); GFX_setTextSize(1);
    GFX_setCursor(68, 120); GFX_printf(line1);
    GFX_setCursor(68, 140); GFX_printf(line2);
}

void platformerUpdate(void)
{
    if (!gameOver && !gameWon && !levelComplete) {
        updatePlayer();
        updateEnemies();
        updateCoins();
        if (playerX + PLAYER_W >= flagX && playerY + PLAYER_H >= GROUND_Y - 82) {
            levelComplete = true;
            score += 300 * (currentLevel + 1);
        }
    }

    drawLevel();
    drawPlayer();
    drawHUD();
    if (gameOver) drawMessage("GAME OVER", "Final score shown above", "Press A to try again", 0xF800);
    else if (gameWon) drawMessage("YOU DID IT!", "All worlds complete", "Press A to play again", 0xFFE0);
    else if (levelComplete) drawMessage("WORLD CLEAR!", "Nice run! Ready for more?", "Press A for next world", 0x07E0);
    GFX_flush();
    sleep_ms(16);
}

