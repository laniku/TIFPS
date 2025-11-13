#include "level.h"
#include <stdlib.h>
#include <string.h>

#define CELL_EMPTY LEVEL_CELL_EMPTY
#define CELL_WALL LEVEL_CELL_WALL
#define CELL_DOOR LEVEL_CELL_DOOR
#define CELL_EXIT LEVEL_CELL_EXIT

static uint8_t map[MAP_WIDTH][MAP_HEIGHT];
static int exit_x = 0;
static int exit_y = 0;
static room_t rooms[MAX_ROOMS];
static int num_rooms = 0;

static uint32_t rng_state = 0;

static uint32_t rng_next(void) {
    rng_state = rng_state * 1103515245 + 12345;
    return (rng_state >> 16) & 0x7FFF;
}

static void generate_room(int x, int y, int w, int h) {
    for (int i = x + 1; i < x + w - 1; i++) {
        for (int j = y + 1; j < y + h - 1; j++) {
            if (i >= 0 && i < MAP_WIDTH && j >= 0 && j < MAP_HEIGHT) {
                map[i][j] = CELL_EMPTY;
            }
        }
    }
}

static void generate_hallway(int x1, int y1, int x2, int y2) {
    int start_x = (x1 < x2) ? x1 : x2;
    int end_x = (x1 < x2) ? x2 : x1;
    for (int x = start_x; x <= end_x; x++) {
        if (x >= 0 && x < MAP_WIDTH && y1 >= 0 && y1 < MAP_HEIGHT) {
            map[x][y1] = CELL_EMPTY;
        }
    }

    int start_y = (y1 < y2) ? y1 : y2;
    int end_y = (y1 < y2) ? y2 : y1;
    for (int y = start_y; y <= end_y; y++) {
        if (x2 >= 0 && x2 < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT) {
            map[x2][y] = CELL_EMPTY;
        }
    }

    if (x1 >= 0 && x1 < MAP_WIDTH && y2 >= 0 && y2 < MAP_HEIGHT) {
        map[x1][y2] = CELL_EMPTY;
    }
    if (x2 >= 0 && x2 < MAP_WIDTH && y1 >= 0 && y1 < MAP_HEIGHT) {
        map[x2][y1] = CELL_EMPTY;
    }
}

void level_init(uint32_t seed) {
    rng_state = seed;
    if (rng_state == 0) rng_state = 1;

    memset(map, CELL_WALL, sizeof(map));

    num_rooms = 4 + (rng_next() % 3);

    for (int i = 0; i < num_rooms; i++) {
        int attempts = 0;
        bool placed = false;

        while (!placed && attempts < 100) {
            rooms[i].w = 3 + (rng_next() % 2);
            rooms[i].h = 3 + (rng_next() % 2);
            rooms[i].x = 2 + (rng_next() % (MAP_WIDTH / 2));
            rooms[i].y = 2 + (rng_next() % (MAP_HEIGHT / 2));

            bool overlap = false;
            for (int j = 0; j < i; j++) {
                if (!(rooms[i].x + rooms[i].w + 1 < rooms[j].x ||
                      rooms[j].x + rooms[j].w + 1 < rooms[i].x ||
                      rooms[i].y + rooms[i].h + 1 < rooms[j].y ||
                      rooms[j].y + rooms[j].h + 1 < rooms[i].y)) {
                    overlap = true;
                    break;
                }
            }

            if (!overlap) {
                generate_room(rooms[i].x, rooms[i].y, rooms[i].w, rooms[i].h);
                rooms[i].center_x = rooms[i].x + rooms[i].w / 2;
                rooms[i].center_y = rooms[i].y + rooms[i].h / 2;
                placed = true;
            }
            attempts++;
        }
    }

    for (int i = 1; i < num_rooms; i++) {
        int prev = i - 1;
        generate_hallway(rooms[prev].center_x, rooms[prev].center_y,
                        rooms[i].center_x, rooms[i].center_y);
    }

    exit_x = rooms[num_rooms - 1].center_x;
    exit_y = rooms[num_rooms - 1].center_y;
    map[exit_x][exit_y] = CELL_EXIT;

    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            int ex = exit_x + dx;
            int ey = exit_y + dy;
            if (ex >= 0 && ex < MAP_WIDTH && ey >= 0 && ey < MAP_HEIGHT) {
                if (map[ex][ey] != CELL_WALL) {
                    map[ex][ey] = CELL_EXIT;
                }
            }
        }
    }

    if (num_rooms > 0) {
        int spawn_x = rooms[0].center_x;
        int spawn_y = rooms[0].center_y;
        map[spawn_x][spawn_y] = CELL_EMPTY;
        map[spawn_x-1][spawn_y] = CELL_EMPTY;
        map[spawn_x+1][spawn_y] = CELL_EMPTY;
        map[spawn_x][spawn_y-1] = CELL_EMPTY;
        map[spawn_x][spawn_y+1] = CELL_EMPTY;
    }
}

void level_cleanup(void) {
}

bool level_is_wall(int x, int y) {
    if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT) {
        return true;
    }
    return map[x][y] == CELL_WALL;
}

bool level_is_at_exit(float x, float y) {
    float dx = x - exit_x;
    float dy = y - exit_y;
    float dist_sq = dx * dx + dy * dy;

    if (dist_sq < 2.25f) {
        return true;
    }
    return false;
}

int level_get_cell(int x, int y) {
    if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT) {
        return CELL_WALL;
    }
    return map[x][y];
}

int level_get_room_count(void) {
    return num_rooms;
}

room_t* level_get_rooms(void) {
    return rooms;
}

uint32_t level_get_rng(void) {
    return rng_state;
}

