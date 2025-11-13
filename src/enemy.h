#ifndef ENEMY_H
#define ENEMY_H

#include <stdbool.h>

#define MAX_ENEMIES 32

typedef enum {
    ENEMY_TYPE_IMP,
    ENEMY_TYPE_DEMON,
    ENEMY_TYPE_BARON
} enemy_type_t;

typedef struct {
    float x, y;
    float angle;
    enemy_type_t type;
    int hp;
    int max_hp;
    bool active;
    float move_timer;
} enemy_t;

void enemy_init(void);
void enemy_update_all(void);
void enemy_cleanup(void);
void enemy_check_shots(void);
int enemy_get_count(void);
enemy_t* enemy_get_list(void);

#endif
