#include "platformer.h"
#include "colors.h"
#include "gfx.h"
#include "pico/stdlib.h"
#include "libs/ownLibs/btnReader.h"
#include "colors.h"
#include <stdint.h>

/* A compact, single-screen platformer for the existing GFX/button API. */
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

/* Separate, seeded PRNG keeps generation readable and avoids a fixed rand() sequence. */
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

/* Makes a guaranteed, jumpable route with controlled variation in height/gaps. */
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

        // Reward exploration above the route; the first few remain reachable from spawn.
        if (i < COIN_MAX - 1) addCoin(x + randomRange(5, width - 12), y - randomRange(18, 30));
    }

    // A few ground coins and enemies keep each roll distinct without blocking the route.
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
    if (btnPressed(BTN_B) || btnPressed(BTN_MENU) || btnPressed(BTN_MENU2)) return 1;

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
    // Small stars/cloud puffs add depth without requiring another graphics API.
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

    // Goal pole is always visible so the player can see where to go.
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


/*#include "platformer.h"
#include "colors.h"
#include "gfx.h"
#include "pico/stdlib.h"
#include "libs/ownLibs/btnReader.h"
#include <stdlib.h>

// Screen dimensions
#define SCREEN_W 320
#define SCREEN_H 240
#define GROUND_Y 200

// Player dimensions
#define PLAYER_W 24
#define PLAYER_H 32

// Physics constants
#define GRAVITY 0.8f
#define JUMP_VELOCITY -12.0f
#define MOVE_SPEED 3.0f

// Level definitions
#define NUM_LEVELS 3
#define PLATFORM_MAX 20

// Platform structure
typedef struct {
    int x, y;
    int w, h;
    int type; // 0=ground, 1=brick, 2=question, 3=pipe
} Platform;

// Enemy structure
typedef struct {
    float x, y;
    float vx;
    int active;
    int type; // 0=goomba
} Enemy;

// Coin structure
typedef struct {
    float x, y;
    int active;
    int frame;
} Coin;

// Game state
static float playerX, playerY;
static float playerVX, playerVY;
static int playerDir; // -1=left, 1=right
static int playerFrame;
static bool isGrounded;
static bool wasGrounded;
static int score;
static int coins;
static int lives;
static int currentLevel;
static bool gameOver;
static bool levelComplete;
static bool gameWon;

static Platform platforms[PLATFORM_MAX];
static int numPlatforms;
static Enemy enemies[5];
static int numEnemies;
static Coin coins_array[10];
static int numCoins;

static int levelFlagX;
static bool flagRaised;
static float flagY;

// Colors for each level
static const uint16_t levelSkyColors[3] = {0x8597, 0x84B0, 0x4460}; // Different sky colors

void initLevel(int level);

void platformerInit(void) {
    playerX = 30;
    playerY = GROUND_Y - PLAYER_H;
    playerVX = 0;
    playerVY = 0;
    playerDir = 1;
    playerFrame = 0;
    isGrounded = false;
    wasGrounded = false;
    score = 0;
    coins = 0;
    lives = 3;
    currentLevel = 0;
    gameOver = false;
    levelComplete = false;
    gameWon = false;
    flagRaised = false;
    flagY = 0;
    
    initLevel(currentLevel);
}

void initLevel(int level) {
    numPlatforms = 0;
    numEnemies = 0;
    numCoins = 0;
    flagRaised = false;
    flagY = 0;
    
    // Ground platform (full width)
    platforms[numPlatforms].x = 0;
    platforms[numPlatforms].y = GROUND_Y;
    platforms[numPlatforms].w = SCREEN_W;
    platforms[numPlatforms].h = SCREEN_H - GROUND_Y;
    platforms[numPlatforms].type = 0; // ground
    numPlatforms++;
    
    if (level == 0) {
        // Level 1: Basic platforms, goombas, coins
        // Platforms
        platforms[numPlatforms].x = 80;
        platforms[numPlatforms].y = 160;
        platforms[numPlatforms].w = 80;
        platforms[numPlatforms].h = 16;
        platforms[numPlatforms].type = 2; // question block
        numPlatforms++;
        
        platforms[numPlatforms].x = 200;
        platforms[numPlatforms].y = 140;
        platforms[numPlatforms].w = 80;
        platforms[numPlatforms].h = 16;
        platforms[numPlatforms].type = 1; // brick
        numPlatforms++;
        
        platforms[numPlatforms].x = 60;
        platforms[numPlatforms].y = 100;
        platforms[numPlatforms].w = 64;
        platforms[numPlatforms].h = 16;
        platforms[numPlatforms].type = 1;
        numPlatforms++;
        
        // Pipe
        platforms[numPlatforms].x = 260;
        platforms[numPlatforms].y = 156;
        platforms[numPlatforms].w = 32;
        platforms[numPlatforms].h = 44;
        platforms[numPlatforms].type = 3; // pipe
        numPlatforms++;
        
        // Enemies
        enemies[numEnemies].x = 150;
        enemies[numEnemies].y = GROUND_Y - 24;
        enemies[numEnemies].vx = -1.5f;
        enemies[numEnemies].active = 1;
        enemies[numEnemies].type = 0;
        numEnemies++;
        
        enemies[numEnemies].x = 230;
        enemies[numEnemies].y = GROUND_Y - 24;
        enemies[numEnemies].vx = -1.0f;
        enemies[numEnemies].active = 1;
        enemies[numEnemies].type = 0;
        numEnemies++;
        
        // Coins
        coins_array[numCoins].x = 110;
        coins_array[numCoins].y = 130;
        coins_array[numCoins].active = 1;
        coins_array[numCoins].frame = 0;
        numCoins++;
        
        coins_array[numCoins].x = 140;
        coins_array[numCoins].y = 130;
        coins_array[numCoins].active = 1;
        coins_array[numCoins].frame = 0;
        numCoins++;
        
        coins_array[numCoins].x = 80;
        coins_array[numCoins].y = 70;
        coins_array[numCoins].active = 1;
        coins_array[numCoins].frame = 0;
        numCoins++;
        
        levelFlagX = 290;
        
    } else if (level == 1) {
        // Level 2: More vertical, more enemies
        platforms[numPlatforms].x = 100;
        platforms[numPlatforms].y = 170;
        platforms[numPlatforms].w = 60;
        platforms[numPlatforms].h = 16;
        platforms[numPlatforms].type = 2;
        numPlatforms++;
        
        platforms[numPlatforms].x = 180;
        platforms[numPlatforms].y = 150;
        platforms[numPlatforms].w = 60;
        platforms[numPlatforms].h = 16;
        platforms[numPlatforms].type = 1;
        numPlatforms++;
        
        platforms[numPlatforms].x = 240;
        platforms[numPlatforms].y = 120;
        platforms[numPlatforms].w = 60;
        platforms[numPlatforms].h = 16;
        platforms[numPlatforms].type = 2;
        numPlatforms++;
        
        platforms[numPlatforms].x = 160;
        platforms[numPlatforms].y = 90;
        platforms[numPlatforms].w = 60;
        platforms[numPlatforms].h = 16;
        platforms[numPlatforms].type = 1;
        numPlatforms++;
        
        platforms[numPlatforms].x = 60;
        platforms[numPlatforms].y = 70;
        platforms[numPlatforms].w = 60;
        platforms[numPlatforms].h = 16;
        platforms[numPlatforms].type = 2;
        numPlatforms++;
        
        // Pipes
        platforms[numPlatforms].x = 30;
        platforms[numPlatforms].y = 156;
        platforms[numPlatforms].w = 32;
        platforms[numPlatforms].h = 44;
        platforms[numPlatforms].type = 3;
        numPlatforms++;
        
        platforms[numPlatforms].x = 280;
        platforms[numPlatforms].y = 156;
        platforms[numPlatforms].w = 32;
        platforms[numPlatforms].h = 44;
        platforms[numPlatforms].type = 3;
        numPlatforms++;
        
        // Enemies
        enemies[numEnemies].x = 120;
        enemies[numEnemies].y = GROUND_Y - 24;
        enemies[numEnemies].vx = -1.2f;
        enemies[numEnemies].active = 1;
        enemies[numEnemies].type = 0;
        numEnemies++;
        
        enemies[numEnemies].x = 200;
        enemies[numEnemies].y = 126;
        enemies[numEnemies].vx = -1.0f;
        enemies[numEnemies].active = 1;
        enemies[numEnemies].type = 0;
        numEnemies++;
        
        enemies[numEnemies].x = 180;
        enemies[numEnemies].y = 66;
        enemies[numEnemies].vx = -0.8f;
        enemies[numEnemies].active = 1;
        enemies[numEnemies].type = 0;
        numEnemies++;
        
        // Coins
        coins_array[numCoins].x = 120;
        coins_array[numCoins].y = 140;
        coins_array[numCoins].active = 1;
        coins_array[numCoins].frame = 0;
        numCoins++;
        
        coins_array[numCoins].x = 260;
        coins_array[numCoins].y = 90;
        coins_array[numCoins].active = 1;
        coins_array[numCoins].frame = 0;
        numCoins++;
        
        coins_array[numCoins].x = 80;
        coins_array[numCoins].y = 40;
        coins_array[numCoins].active = 1;
        coins_array[numCoins].frame = 0;
        numCoins++;
        
        levelFlagX = 290;
        
    } else {
        // Level 3: Hardest - more enemies, trickier jumps
        platforms[numPlatforms].x = 50;
        platforms[numPlatforms].y = 175;
        platforms[numPlatforms].w = 50;
        platforms[numPlatforms].h = 16;
        platforms[numPlatforms].type = 2;
        numPlatforms++;
        
        platforms[numPlatforms].x = 130;
        platforms[numPlatforms].y = 155;
        platforms[numPlatforms].w = 50;
        platforms[numPlatforms].h = 16;
        platforms[numPlatforms].type = 1;
        numPlatforms++;
        
        platforms[numPlatforms].x = 200;
        platforms[numPlatforms].y = 135;
        platforms[numPlatforms].w = 50;
        platforms[numPlatforms].h = 16;
        platforms[numPlatforms].type = 2;
        numPlatforms++;
        
        platforms[numPlatforms].x = 260;
        platforms[numPlatforms].y = 115;
        platforms[numPlatforms].w = 50;
        platforms[numPlatforms].h = 16;
        platforms[numPlatforms].type = 1;
        numPlatforms++;
        
        platforms[numPlatforms].x = 180;
        platforms[numPlatforms].y = 85;
        platforms[numPlatforms].w = 50;
        platforms[numPlatforms].h = 16;
        platforms[numPlatforms].type = 2;
        numPlatforms++;
        
        platforms[numPlatforms].x = 100;
        platforms[numPlatforms].y = 65;
        platforms[numPlatforms].w = 50;
        platforms[numPlatforms].h = 16;
        platforms[numPlatforms].type = 1;
        numPlatforms++;
        
        // Multiple pipes
        platforms[numPlatforms].x = 10;
        platforms[numPlatforms].y = 156;
        platforms[numPlatforms].w = 32;
        platforms[numPlatforms].h = 44;
        platforms[numPlatforms].type = 3;
        numPlatforms++;
        
        platforms[numPlatforms].x = 280;
        platforms[numPlatforms].y = 156;
        platforms[numPlatforms].w = 32;
        platforms[numPlatforms].h = 44;
        platforms[numPlatforms].type = 3;
        numPlatforms++;
        
        // Enemies - more aggressive
        enemies[numEnemies].x = 80;
        enemies[numEnemies].y = GROUND_Y - 24;
        enemies[numEnemies].vx = -1.8f;
        enemies[numEnemies].active = 1;
        enemies[numEnemies].type = 0;
        numEnemies++;
        
        enemies[numEnemies].x = 160;
        enemies[numEnemies].y = 131;
        enemies[numEnemies].vx = -1.5f;
        enemies[numEnemies].active = 1;
        enemies[numEnemies].type = 0;
        numEnemies++;
        
        enemies[numEnemies].x = 220;
        enemies[numEnemies].y = GROUND_Y - 24;
        enemies[numEnemies].vx = -1.3f;
        enemies[numEnemies].active = 1;
        enemies[numEnemies].type = 0;
        numEnemies++;
        
        enemies[numEnemies].x = 200;
        enemies[numEnemies].y = 61;
        enemies[numEnemies].vx = -1.0f;
        enemies[numEnemies].active = 1;
        enemies[numEnemies].type = 0;
        numEnemies++;
        
        // Coins scattered
        coins_array[numCoins].x = 65;
        coins_array[numCoins].y = 145;
        coins_array[numCoins].active = 1;
        coins_array[numCoins].frame = 0;
        numCoins++;
        
        coins_array[numCoins].x = 145;
        coins_array[numCoins].y = 125;
        coins_array[numCoins].active = 1;
        coins_array[numCoins].frame = 0;
        numCoins++;
        
        coins_array[numCoins].x = 215;
        coins_array[numCoins].y = 105;
        coins_array[numCoins].active = 1;
        coins_array[numCoins].frame = 0;
        numCoins++;
        
        coins_array[numCoins].x = 275;
        coins_array[numCoins].y = 85;
        coins_array[numCoins].active = 1;
        coins_array[numCoins].frame = 0;
        numCoins++;
        
        levelFlagX = 290;
    }
    
    // Reset player position
    playerX = 30;
    playerY = GROUND_Y - PLAYER_H;
    playerVX = 0;
    playerVY = 0;
}

int platformerRead(void) {
    if (btnPressed(BTN_B) || btnPressed(BTN_MENU) || btnPressed(BTN_MENU2)) {
        return 1;
    }
    
    if (gameOver || gameWon) {
        if (btnPressed(BTN_A)) {
            platformerInit();
        }
        return 0;
    }
    
    if (levelComplete) {
        if (btnPressed(BTN_A)) {
            currentLevel++;
            if (currentLevel >= NUM_LEVELS) {
                gameWon = true;
            } else {
                initLevel(currentLevel);
                levelComplete = false;
            }
        }
        return 0;
    }
    
    // Movement
    playerVX = 0;
    if (btnDown(BTN_LEFT)) {
        playerVX = -MOVE_SPEED;
        playerDir = -1;
    }
    if (btnDown(BTN_RIGHT)) {
        playerVX = MOVE_SPEED;
        playerDir = 1;
    }
    
    // Jump
    if ((btnPressed(BTN_UP) || btnPressed(BTN_A)) && isGrounded) {
        playerVY = JUMP_VELOCITY;
        isGrounded = false;
    }
    
    return 0;
}

static bool checkPlatformCollision(float newX, float newY) {
    int pw = PLAYER_W;
    int ph = PLAYER_H;
    
    for (int i = 0; i < numPlatforms; i++) {
        Platform *p = &platforms[i];
        
        if (newX + pw > p->x && newX < p->x + p->w &&
            newY + ph > p->y && newY < p->y + p->h) {
            return true;
        }
    }
    return false;
}

static void updatePlayer(void) {
    wasGrounded = isGrounded;
    
    // Apply gravity
    playerVY += GRAVITY;
    if (playerVY > 15) playerVY = 15;
    
    // Move X
    float newX = playerX + playerVX;
    if (newX >= 0 && newX + PLAYER_W <= SCREEN_W && !checkPlatformCollision(newX, playerY)) {
        playerX = newX;
    }
    
    // Move Y
    float newY = playerY + playerVY;
    isGrounded = false;
    
    if (checkPlatformCollision(playerX, newY)) {
        if (playerVY > 0) {
            // Landing on top
            for (int i = 0; i < numPlatforms; i++) {
                Platform *p = &platforms[i];
                if (playerX + PLAYER_W > p->x && playerX < p->x + p->w) {
                    if (playerY + PLAYER_H <= p->y + 5 && playerY + PLAYER_H + playerVY >= p->y) {
                        newY = p->y - PLAYER_H;
                        isGrounded = true;
                        playerVY = 0;
                        break;
                    }
                }
            }
        } else if (playerVY < 0) {
            // Hitting head
            for (int i = 0; i < numPlatforms; i++) {
                Platform *p = &platforms[i];
                if (playerX + PLAYER_W > p->x && playerX < p->x + p->w) {
                    if (playerY >= p->y + p->h - 5 && playerY + playerVY <= p->y + p->h) {
                        newY = p->y + p->h;
                        playerVY = 0;
                        break;
                    }
                }
            }
        }
    }
    
    playerY = newY;
    
    // Fall off screen
    if (playerY > SCREEN_H) {
        lives--;
        if (lives <= 0) {
            gameOver = true;
        } else {
            playerX = 30;
            playerY = GROUND_Y - PLAYER_H;
            playerVX = 0;
            playerVY = 0;
        }
    }
    
    // Animate player
    if (playerVX != 0) {
        playerFrame = (playerFrame + 1) % 4;
    } else {
        playerFrame = 0;
    }
}

static void updateEnemies(void) {
    for (int i = 0; i < numEnemies; i++) {
        if (!enemies[i].active) continue;
        
        // Move enemy
        enemies[i].x += enemies[i].vx;
        
        // Reverse at edges or platforms
        int onPlatform = 0;
        for (int j = 0; j < numPlatforms; j++) {
            Platform *p = &platforms[j];
            if (enemies[i].x + 24 > p->x && enemies[i].x < p->x + p->w &&
                enemies[i].y + 24 >= p->y - 2 && enemies[i].y + 24 <= p->y + 10) {
                onPlatform = 1;
                // Check edge
                if (enemies[i].x <= p->x || enemies[i].x + 24 >= p->x + p->w) {
                    enemies[i].vx *= -1;
                }
                break;
            }
        }
        
        if (enemies[i].x < 0 || enemies[i].x > SCREEN_W - 24) {
            enemies[i].vx *= -1;
        }
        
        // Check collision with player
        if (playerX + PLAYER_W > enemies[i].x && playerX < enemies[i].x + 24 &&
            playerY + PLAYER_H > enemies[i].y && playerY < enemies[i].y + 24) {
            
            // Stomp from above
            if (playerVY > 0 && playerY + PLAYER_H < enemies[i].y + 16) {
                enemies[i].active = 0;
                score += 100;
                playerVY = -8; // Bounce
            } else {
                // Player hit
                lives--;
                if (lives <= 0) {
                    gameOver = true;
                } else {
                    playerX = 30;
                    playerY = GROUND_Y - PLAYER_H;
                    playerVX = 0;
                    playerVY = 0;
                }
            }
        }
        
        // Fall off screen
        if (enemies[i].y > SCREEN_H) {
            enemies[i].active = 0;
        }
    }
}

static void updateCoins(void) {
    for (int i = 0; i < numCoins; i++) {
        if (!coins_array[i].active) continue;
        
        // Animate
        coins_array[i].frame = (coins_array[i].frame + 1) % 6;
        
        // Check collision
        if (playerX + PLAYER_W > coins_array[i].x && playerX < coins_array[i].x + 16 &&
            playerY + PLAYER_H > coins_array[i].y && playerY < coins_array[i].y + 16) {
            coins_array[i].active = 0;
            coins++;
            score += 50;
        }
    }
}

static void checkFlag(void) {
    // Check if player reached flag
    if (playerX + PLAYER_W > levelFlagX && playerX < levelFlagX + 16 &&
        playerY + PLAYER_H > GROUND_Y - 80 && playerY < GROUND_Y) {
        levelComplete = true;
        score += 500 * (currentLevel + 1);
    }
}

static void drawLevel(void) {
    // Sky background
    GFX_fillScreen(levelSkyColors[currentLevel]);
    
    // Draw platforms
    for (int i = 0; i < numPlatforms; i++) {
        Platform *p = &platforms[i];
        
        if (p->type == 0) {
            // Ground - draw as blocks
            for (int bx = p->x; bx < p->x + p->w; bx += 16) {
                for (int by = p->y; by < p->y + p->h; by += 16) {
                    draw_image_scaled(bx, by, 16, 16, 1, ground_16x16);
                }
            }
        } else if (p->type == 1) {
            // Brick
            for (int bx = p->x; bx < p->x + p->w; bx += 16) {
                draw_image_scaled(bx, p->y, 16, 16, 1, brick_16x16);
            }
        } else if (p->type == 2) {
            // Question block
            for (int bx = p->x; bx < p->x + p->w; bx += 16) {
                draw_image_scaled(bx, p->y, 16, 16, 1, question_16x16);
            }
        } else if (p->type == 3) {
            // Pipe
            draw_image_scaled(p->x, p->y, 16, 32, 2, pipe_16x32);
        }
    }
    
    // Draw flag
    if (levelComplete && flagY < 48) {
        flagY += 2;
    }
    if (flagY > 0) {
        // Pole
        GFX_fillRect(levelFlagX + 4, GROUND_Y - 80, 4, 80, GRAY);
        // Flag
        draw_image_scaled(levelFlagX, GROUND_Y - 80 - (int)flagY, 8, 48, 1, flag_8x48);
    }
    
    // Draw enemies
    for (int i = 0; i < numEnemies; i++) {
        if (!enemies[i].active) continue;
        draw_image_scaled((int)enemies[i].x, (int)enemies[i].y, 16, 16, 1.5f, goomba_16x16);
    }
    
    // Draw coins
    for (int i = 0; i < numCoins; i++) {
        if (!coins_array[i].active) continue;
        int offset = (coins_array[i].frame < 3) ? 0 : 4;
        draw_image_scaled((int)coins_array[i].x, (int)coins_array[i].y + offset, 16, 16, 1, coin_16x16);
    }
}

static void drawPlayer(void) {
    // Draw mario sprite (flip based on direction)
    // For simplicity, we just draw the mario sprite scaled up
    if (playerDir > 0) {
        draw_image_scaled((int)playerX, (int)playerY, 16, 16, 1.5f, mario_16x16);
    } else {
        // Flip horizontally by drawing mirrored - for now just draw normally
        // In a full implementation, we'd use sprite flipping
        draw_image_scaled((int)playerX, (int)playerY, 16, 16, 1.5f, mario_16x16);
    }
}

static void drawHUD(void) {
    // HUD background
    GFX_fillRect(0, 0, SCREEN_W, 24, BLACK);
    
    GFX_setTextColor(WHITE);
    GFX_setTextBack(BLACK);
    GFX_setTextSize(1);
    
    GFX_setCursor(8, 6);
    GFX_printf("LVL%d", currentLevel + 1);
    
    GFX_setCursor(60, 6);
    GFX_printf("SCORE:%d", score);
    
    GFX_setCursor(180, 6);
    GFX_printf("COINS:%d", coins);
    
    GFX_setCursor(260, 6);
    GFX_printf("LIVES:%d", lives);
}

void platformerUpdate(void) {
    if (gameOver || gameWon || levelComplete) {
        sleep_ms(100);
        
        // Draw overlays
        drawLevel();
        drawHUD();
        
        if (gameOver) {
            GFX_fillRect(60, 80, 200, 80, BLACK);
            GFX_drawRect(60, 80, 200, 80, RED);
            GFX_setTextColor(RED);
            GFX_setTextSize(2);
            GFX_setCursor(90, 95);
            GFX_printf("GAME OVER");
            GFX_setTextColor(WHITE);
            GFX_setTextSize(1);
            GFX_setCursor(85, 125);
            GFX_printf("Score: %d", score);
            GFX_setCursor(85, 140);
            GFX_printf("Press A to Restart");
            GFX_flush();
            return;
        }
        
        if (gameWon) {
            GFX_fillRect(50, 70, 220, 100, BLACK);
            GFX_drawRect(50, 70, 220, 100, GOLD);
            GFX_setTextColor(GOLD);
            GFX_setTextSize(2);
            GFX_setCursor(95, 80);
            GFX_printf("YOU WIN!");
            GFX_setTextColor(WHITE);
            GFX_setTextSize(1);
            GFX_setCursor(85, 110);
            GFX_printf("Final Score: %d", score);
            GFX_setCursor(70, 125);
            GFX_printf("Coins: %d", coins);
            GFX_setCursor(80, 145);
            GFX_printf("Press A to Play Again");
            GFX_flush();
            return;
        }
        
        if (levelComplete) {
            GFX_fillRect(50, 80, 220, 80, BLACK);
            GFX_drawRect(50, 80, 220, 80, GREEN);
            GFX_setTextColor(GREEN);
            GFX_setTextSize(2);
            GFX_setCursor(90, 90);
            GFX_printf("LEVEL CLEAR!");
            GFX_setTextColor(WHITE);
            GFX_setTextSize(1);
            GFX_setCursor(85, 120);
            GFX_printf("Score: %d", score);
            GFX_setCursor(85, 135);
            GFX_printf("Press A for Next Level");
            GFX_flush();
            return;
        }
    }
    
    updatePlayer();
    updateEnemies();
    updateCoins();
    checkFlag();
    
    drawLevel();
    drawPlayer();
    drawHUD();
    
    GFX_flush();
    sleep_ms(16); // ~60 FPS
}*/