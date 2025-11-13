#ifndef PLAYER_H
#define PLAYER_H

#include <stdbool.h>

void player_init(void);
void player_update(void);
float player_get_x(void);
float player_get_y(void);
float player_get_angle(void);
int player_get_hp(void);
int player_get_ammo(void);
void player_take_damage(int damage);
void player_add_ammo(int amount);
bool player_shoot(void);
bool player_is_shooting(void);

#endif
