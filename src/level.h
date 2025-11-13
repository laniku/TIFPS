#ifndef LEVEL_H
#define LEVEL_H

#include <stdbool.h>
#include <stdint.h>

#define LEVEL_CELL_EMPTY 0
#define LEVEL_CELL_WALL 1
#define LEVEL_CELL_DOOR 2
#define LEVEL_CELL_EXIT 3

#define MAP_WIDTH 24
#define MAP_HEIGHT 24
#define MAX_ROOMS 12

typedef struct {
    int x, y, w, h;
    int center_x, center_y;
} room_t;

void level_init(uint32_t seed);
void level_cleanup(void);
bool level_is_wall(int x, int y);
bool level_is_at_exit(float x, float y);
int level_get_cell(int x, int y);
int level_get_room_count(void);
room_t* level_get_rooms(void);
uint32_t level_get_rng(void);

#endif
