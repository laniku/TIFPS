#include "game.h"
#include "player.h"
#include "level.h"
#include "enemy.h"
#include "ui.h"
#include "raycast.h"
#include <keypadc.h>
#include <time.h>

static bool running = false;
static bool initialized = false;

void game_init(void) {
    if (initialized) return;

    time_t t = time(NULL);
    uint32_t seed = (uint32_t)t;
    if (seed == 0) seed = 12345;

    level_init(seed);
    player_init();
    enemy_init();
    ui_init();
    raycast_init();

    running = true;
    initialized = true;
}

void game_update(void) {
    if (!running) return;

    kb_Scan();
    if (kb_Data[6] & kb_Clear) {
        running = false;
        return;
    }

    player_update();
    enemy_update_all();

    if (level_is_at_exit(player_get_x(), player_get_y())) {
        running = false;
    }

    if (player_get_hp() <= 0) {
        running = false;
    }
}

void game_render(void) {
    if (!running) return;

    raycast_render();
    raycast_render_enemies();
    ui_render();
    ui_render_crosshair();
}

void game_cleanup(void) {
    enemy_cleanup();
    level_cleanup();
    initialized = false;
}

bool game_is_running(void) {
    return running;
}

