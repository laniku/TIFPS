#include "player.h"
#include "level.h"
#include "enemy.h"
#include <keypadc.h>
#include <math.h>
#include <stdbool.h>

#define MOVE_SPEED 0.05
#define ROT_SPEED 0.05
#define MAX_HP 100
#define MAX_AMMO 50
#define START_AMMO 30

static float px = 1.5;
static float py = 1.5;
static float angle = 0.0;
static int hp = MAX_HP;
static int ammo = START_AMMO;
static bool shoot_pressed = false;
static bool is_shooting = false;
static int shoot_timer = 0;

void player_init(void) {
    int room_count = level_get_room_count();
    if (room_count > 0) {
        room_t* rooms = level_get_rooms();
        px = rooms[0].center_x + 0.5;
        py = rooms[0].center_y + 0.5;
    } else {
        px = 1.5;
        py = 1.5;
    }

    angle = 0.0;
    hp = MAX_HP;
    ammo = START_AMMO;
    shoot_pressed = false;

    if (level_is_wall((int)px, (int)py)) {
        px += 0.5;
        py += 0.5;
    }
}

void player_update(void) {
    kb_Scan();

    float new_x = px;
    float new_y = py;
    float new_angle = angle;

    if (kb_Data[7] & kb_Left) {
        new_angle += ROT_SPEED;
    }
    if (kb_Data[7] & kb_Right) {
        new_angle -= ROT_SPEED;
    }

    while (new_angle < 0) new_angle += 2 * M_PI;
    while (new_angle >= 2 * M_PI) new_angle -= 2 * M_PI;
    angle = new_angle;

    if (kb_Data[7] & kb_Up) {
        new_x += cosf(angle) * MOVE_SPEED;
        new_y += sinf(angle) * MOVE_SPEED;
    }
    if (kb_Data[7] & kb_Down) {
        new_x -= cosf(angle) * MOVE_SPEED;
        new_y -= sinf(angle) * MOVE_SPEED;
    }

    if (!level_is_wall((int)new_x, (int)new_y) &&
        !level_is_wall((int)(new_x + 0.2), (int)new_y) &&
        !level_is_wall((int)new_x, (int)(new_y + 0.2)) &&
        !level_is_wall((int)(new_x - 0.2), (int)new_y) &&
        !level_is_wall((int)new_x, (int)(new_y - 0.2))) {
        px = new_x;
        py = new_y;
    }

    bool shoot_key = kb_Data[1] & kb_2nd;
    if (shoot_key && !shoot_pressed && ammo > 0) {
        if (player_shoot()) {
            enemy_check_shots();
            is_shooting = true;
            shoot_timer = 3;
        }
        shoot_pressed = true;
    } else if (!shoot_key) {
        shoot_pressed = false;
    }

    if (shoot_timer > 0) {
        shoot_timer--;
        if (shoot_timer == 0) {
            is_shooting = false;
        }
    }
}

bool player_is_shooting(void) {
    return is_shooting;
}

float player_get_x(void) {
    return px;
}

float player_get_y(void) {
    return py;
}

float player_get_angle(void) {
    return angle;
}

int player_get_hp(void) {
    return hp;
}

int player_get_ammo(void) {
    return ammo;
}

void player_take_damage(int damage) {
    hp -= damage;
    if (hp < 0) hp = 0;
}

void player_add_ammo(int amount) {
    ammo += amount;
    if (ammo > MAX_AMMO) ammo = MAX_AMMO;
}

bool player_shoot(void) {
    if (ammo <= 0) return false;

    ammo--;
    return true;
}

