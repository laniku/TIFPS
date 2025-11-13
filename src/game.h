#ifndef GAME_H
#define GAME_H

#include <stdbool.h>

void game_init(void);
void game_update(void);
void game_render(void);
void game_cleanup(void);
bool game_is_running(void);

#endif
