#include "enemy.h"
#include "player.h"
#include "level.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

static uint32_t enemy_rng_state = 0;

static uint32_t enemy_rng_next(void) {
    enemy_rng_state = enemy_rng_state * 1103515245 + 12345;
    return (enemy_rng_state >> 16) & 0x7FFF;
}

static enemy_t enemies[MAX_ENEMIES];
static int enemy_count = 0;

static const int enemy_hp[] = {20, 40, 80};
static const float enemy_speed[] = {0.03, 0.02, 0.015};
static const float enemy_damage[] = {5, 10, 15};
static const float enemy_range[] = {8.0, 10.0, 12.0};

static void spawn_enemy(float x, float y, enemy_type_t type) {
    if (enemy_count >= MAX_ENEMIES) return;

    enemy_t* e = &enemies[enemy_count++];
    e->x = x;
    e->y = y;
    e->angle = 0.0;
    e->type = type;
    e->hp = enemy_hp[type];
    e->max_hp = enemy_hp[type];
    e->active = true;
    e->move_timer = 0.0;
}

void enemy_init(void) {
    memset(enemies, 0, sizeof(enemies));
    enemy_count = 0;

    enemy_rng_state = level_get_rng();

    int room_count = level_get_room_count();
    room_t* rooms = level_get_rooms();

    for (int i = 1; i < room_count - 1 && enemy_count < MAX_ENEMIES; i++) {
        int enemies_per_room = 1 + (enemy_rng_next() % 3);

        for (int j = 0; j < enemies_per_room && enemy_count < MAX_ENEMIES; j++) {
            int room_w = rooms[i].w - 2;
            int room_h = rooms[i].h - 2;
            if (room_w > 0 && room_h > 0) {
                float spawn_x = rooms[i].x + 1.0 + (enemy_rng_next() % room_w);
                float spawn_y = rooms[i].y + 1.0 + (enemy_rng_next() % room_h);

                enemy_type_t type = (enemy_type_t)(enemy_rng_next() % 3);

                spawn_enemy(spawn_x, spawn_y, type);
            }
        }
    }

    if (enemy_count == 0 && room_count > 2) {
        int mid_room = room_count / 2;
        if (mid_room > 0 && mid_room < room_count - 1) {
            spawn_enemy(rooms[mid_room].center_x + 0.5,
                       rooms[mid_room].center_y + 0.5, ENEMY_TYPE_IMP);
        }
    }
}

void enemy_update_all(void) {
    float px = player_get_x();
    float py = player_get_y();

    for (int i = 0; i < enemy_count; i++) {
        enemy_t* e = &enemies[i];
        if (!e->active) continue;

        float dx = px - e->x;
        float dy = py - e->y;
        float dist_sq = dx * dx + dy * dy;
        float dist = sqrtf(dist_sq);

        e->angle = atan2f(dy, dx);

        float range_sq = enemy_range[e->type] * enemy_range[e->type];
        if (dist_sq < range_sq && dist > 0.3f) {
            float speed = enemy_speed[e->type];
            float cos_angle = cosf(e->angle);
            float sin_angle = sinf(e->angle);
            float new_x = e->x + cos_angle * speed;
            float new_y = e->y + sin_angle * speed;

            int new_xi = (int)new_x;
            int new_yi = (int)new_y;
            if (new_x < 0.0f) new_xi--;
            if (new_y < 0.0f) new_yi--;

            if (!level_is_wall(new_xi, new_yi)) {
                e->x = new_x;
                e->y = new_y;
            }
        }

        if (dist < 1.0) {
            e->move_timer += 0.2;
            if (e->move_timer > 0.5) {
                player_take_damage((int)enemy_damage[e->type]);
                e->move_timer = 0.0;
            }
        } else {
            e->move_timer = 0.0;
        }
    }
}

void enemy_check_shots(void) {
    float px = player_get_x();
    float py = player_get_y();
    float pangle = player_get_angle();

    for (int i = 0; i < enemy_count; i++) {
        enemy_t* e = &enemies[i];
        if (!e->active) continue;

        float dx = e->x - px;
        float dy = e->y - py;
        float dist_sq = dx * dx + dy * dy;

        if (dist_sq < 64.0f) {
            float angle_to_enemy = atan2f(dy, dx);
            float angle_diff = angle_to_enemy - pangle;

            while (angle_diff > M_PI) angle_diff -= 2 * M_PI;
            while (angle_diff < -M_PI) angle_diff += 2 * M_PI;

            if (fabsf(angle_diff) < M_PI / 3.0f) {
                e->hp -= 10;
                if (e->hp <= 0) {
                    e->active = false;
                    if ((enemy_rng_next() % 3) == 0) {
                        player_add_ammo(5);
                    }
                }
            }
        }
    }
}

void enemy_cleanup(void) {
    enemy_count = 0;
}

int enemy_get_count(void) {
    return enemy_count;
}

enemy_t* enemy_get_list(void) {
    return enemies;
}

