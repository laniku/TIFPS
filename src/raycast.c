#include "raycast.h"
#include "player.h"
#include "level.h"
#include "enemy.h"
#include <graphx.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define FOV (M_PI / 3.0f)
#define MAX_DEPTH 20.0f
#define RAY_STEP 5

#define COLOR_BLACK 0
#define COLOR_SKY_NEAR 1
#define COLOR_SKY_MID 2
#define COLOR_SKY_FAR 3
#define COLOR_FLOOR_NEAR 4
#define COLOR_FLOOR_MID 5
#define COLOR_FLOOR_FAR 6
#define COLOR_WALL_LIGHT 7
#define COLOR_WALL_MEDIUM 8
#define COLOR_WALL_DARK 9
#define COLOR_WALL_SHADE_LIGHT 10
#define COLOR_WALL_SHADE_MEDIUM 11
#define COLOR_WALL_SHADE_DARK 12
#define COLOR_DOOR_LIGHT 13
#define COLOR_DOOR_MID 14
#define COLOR_DOOR_DARK 15
#define COLOR_EXIT_LIGHT 16
#define COLOR_EXIT_MID 17
#define COLOR_EXIT_DARK 18
#define COLOR_ENEMY 19
#define COLOR_CROSSHAIR 20
#define COLOR_WEAPON_PRIMARY 21
#define COLOR_WEAPON_ACCENT 22
#define PALETTE_ENTRY_COUNT 23

typedef struct {
    uint8_t near;
    uint8_t mid;
    uint8_t far;
} shade_triplet_t;

static float depth_buffer[SCREEN_WIDTH];
static float depth_template[SCREEN_WIDTH];

static float cos_lookup[SCREEN_WIDTH];
static float sin_lookup[SCREEN_WIDTH];

static uint16_t palette_data[PALETTE_ENTRY_COUNT];
static bool palette_ready = false;

static const shade_triplet_t wall_shades[2] = {
    { COLOR_WALL_LIGHT, COLOR_WALL_MEDIUM, COLOR_WALL_DARK },
    { COLOR_WALL_SHADE_LIGHT, COLOR_WALL_SHADE_MEDIUM, COLOR_WALL_SHADE_DARK }
};

static const shade_triplet_t door_shades = {
    COLOR_DOOR_LIGHT, COLOR_DOOR_MID, COLOR_DOOR_DARK
};

static const shade_triplet_t exit_shades = {
    COLOR_EXIT_LIGHT, COLOR_EXIT_MID, COLOR_EXIT_DARK
};

static const shade_triplet_t floor_shades = {
    COLOR_FLOOR_NEAR, COLOR_FLOOR_MID, COLOR_FLOOR_FAR
};

static const shade_triplet_t ceiling_shades = {
    COLOR_SKY_NEAR, COLOR_SKY_MID, COLOR_SKY_FAR
};

static inline uint8_t select_shade(const shade_triplet_t *set, float dist) {
    if (dist < 4.0f) {
        return set->near;
    }
    if (dist < 10.0f) {
        return set->mid;
    }
    return set->far;
}

static inline void draw_segment(int x, int y1, int y2, uint8_t color) {
    if (y2 < y1) {
        return;
    }
    gfx_SetColor(color);
    gfx_VertLine(x, y1, y2);
}

static inline void depth_buffer_reset(void) {
    memcpy(depth_buffer, depth_template, sizeof(depth_buffer));
}

void raycast_render(void) {
    float px = player_get_x();
    float py = player_get_y();
    float angle = player_get_angle();

    depth_buffer_reset();

    gfx_SetColor(COLOR_SKY_FAR);
    gfx_FillScreen(COLOR_SKY_FAR);

    float cos_angle = cosf(angle);
    float sin_angle = sinf(angle);

    const int max_steps = (int)(MAX_DEPTH * 4.0f);
    const int screen_half = SCREEN_HEIGHT / 2;

    for (int x = 0; x < SCREEN_WIDTH; x += RAY_STEP) {
        int sample = x + (RAY_STEP / 2);
        if (sample >= SCREEN_WIDTH) {
            sample = SCREEN_WIDTH - 1;
        }

        float rel_cos = cos_lookup[sample];
        float rel_sin = sin_lookup[sample];

        float dx = cos_angle * rel_cos - sin_angle * rel_sin;
        float dy = sin_angle * rel_cos + cos_angle * rel_sin;

        float step_x = (dx > 0.0f) ? 1.0f : -1.0f;
        float step_y = (dy > 0.0f) ? 1.0f : -1.0f;

        float delta_x = (fabsf(dx) > 1e-6f) ? fabsf(1.0f / dx) : 1e30f;
        float delta_y = (fabsf(dy) > 1e-6f) ? fabsf(1.0f / dy) : 1e30f;

        int map_x = (int)px;
        int map_y = (int)py;
        if (px < 0.0f) map_x--;
        if (py < 0.0f) map_y--;

        float side_dist_x = (dx < 0.0f)
                                ? (px - (float)map_x) * delta_x
                                : ((float)map_x + 1.0f - px) * delta_x;
        float side_dist_y = (dy < 0.0f)
                                ? (py - (float)map_y) * delta_y
                                : ((float)map_y + 1.0f - py) * delta_y;

        int map_xi = map_x;
        int map_yi = map_y;
        bool hit = false;
        bool hit_vertical = false;
        int hit_cell = LEVEL_CELL_WALL;
        float dist = MAX_DEPTH;

        for (int step = 0; step < max_steps; step++) {
            if (side_dist_x < side_dist_y) {
                side_dist_x += delta_x;
                map_xi += (int)step_x;
                hit_vertical = false;
            } else {
                side_dist_y += delta_y;
                map_yi += (int)step_y;
                hit_vertical = true;
            }

            int cell = level_get_cell(map_xi, map_yi);
            if (cell == LEVEL_CELL_WALL || cell == LEVEL_CELL_DOOR || cell == LEVEL_CELL_EXIT) {
                hit = true;
                hit_cell = cell;
                float map_xf = (float)map_xi;
                float map_yf = (float)map_yi;
                if (hit_vertical) {
                    dist = (map_yf - py + (1.0f - step_y) * 0.5f) / dy;
                } else {
                    dist = (map_xf - px + (1.0f - step_x) * 0.5f) / dx;
                }
                break;
            }
        }

        float perp_dist = MAX_DEPTH;
        bool has_wall = false;
        int draw_start = screen_half;
        int draw_end = screen_half - 1;
        uint8_t wall_color = COLOR_WALL_MEDIUM;

        if (hit && dist > 0.0f && dist < MAX_DEPTH) {
            perp_dist = fabsf(dist * rel_cos);
            if (perp_dist < 0.001f) {
                perp_dist = 0.001f;
            }

            int line_height = (int)(SCREEN_HEIGHT / perp_dist);
            if (line_height < 1) line_height = 1;

            draw_start = screen_half - (line_height / 2);
            draw_end = draw_start + line_height - 1;

            if (draw_start < 0) draw_start = 0;
            if (draw_end >= SCREEN_HEIGHT) draw_end = SCREEN_HEIGHT - 1;

            const shade_triplet_t *shade_set = &wall_shades[hit_vertical ? 1 : 0];
            if (hit_cell == LEVEL_CELL_DOOR) {
                shade_set = &door_shades;
            } else if (hit_cell == LEVEL_CELL_EXIT) {
                shade_set = &exit_shades;
            }

            wall_color = select_shade(shade_set, perp_dist);
            has_wall = true;
        }

        int ceiling_end = has_wall ? draw_start - 1 : screen_half - 1;
        uint8_t ceiling_color = select_shade(&ceiling_shades, has_wall ? perp_dist : MAX_DEPTH);
        uint8_t floor_color = select_shade(&floor_shades, has_wall ? perp_dist : MAX_DEPTH);

        for (int col = 0; col < RAY_STEP && (x + col) < SCREEN_WIDTH; col++) {
            int column = x + col;

            if (has_wall) {
                depth_buffer[column] = perp_dist;
            } else {
                depth_buffer[column] = MAX_DEPTH;
            }

            if (ceiling_end >= 0) {
                draw_segment(column, 0, ceiling_end, ceiling_color);
            }

            if (has_wall) {
                draw_segment(column, draw_start, draw_end, wall_color);
            }

            int floor_start = has_wall ? draw_end + 1 : screen_half;
            if (floor_start < SCREEN_HEIGHT) {
                draw_segment(column, floor_start, SCREEN_HEIGHT - 1, floor_color);
            }
        }
    }
}

void raycast_render_enemies(void) {
    float px = player_get_x();
    float py = player_get_y();
    float pangle = player_get_angle();

    float cos_angle = cosf(-pangle);
    float sin_angle = sinf(-pangle);

    enemy_t *enemies = enemy_get_list();
    int enemy_count = enemy_get_count();

    for (int i = 0; i < enemy_count; i++) {
        enemy_t *e = &enemies[i];
        if (!e->active) {
            continue;
        }

        float dx = e->x - px;
        float dy = e->y - py;

        float transform_x = dx * cos_angle - dy * sin_angle;
        float transform_y = dx * sin_angle + dy * cos_angle;

        if (transform_y <= 0.1f || transform_y >= MAX_DEPTH) {
            continue;
        }

        float inv_transform_y = 1.0f / transform_y;
        int sprite_screen_x = (int)((SCREEN_WIDTH / 2) * (1.0f + transform_x * inv_transform_y));

        int sprite_height = (int)(SCREEN_HEIGHT * inv_transform_y);
        int sprite_width = sprite_height;

        int half_width = sprite_width / 2;
        if (sprite_screen_x + half_width < 0 || sprite_screen_x - half_width >= SCREEN_WIDTH) {
            continue;
        }

        int check_x = sprite_screen_x;
        if (check_x < 0) check_x = 0;
        if (check_x >= SCREEN_WIDTH) check_x = SCREEN_WIDTH - 1;

        if (transform_y > depth_buffer[check_x] + 0.1f) {
            continue;
        }

        int draw_start_x = sprite_screen_x - sprite_width / 2;
        int draw_end_x = sprite_screen_x + sprite_width / 2;
        int draw_start_y = (SCREEN_HEIGHT / 2) - sprite_height / 2;
        int draw_end_y = (SCREEN_HEIGHT / 2) + sprite_height / 2;

        if (draw_start_x < 0) draw_start_x = 0;
        if (draw_end_x >= SCREEN_WIDTH) draw_end_x = SCREEN_WIDTH - 1;
        if (draw_start_y < 0) draw_start_y = 0;
        if (draw_end_y >= SCREEN_HEIGHT) draw_end_y = SCREEN_HEIGHT - 1;

        int width = draw_end_x - draw_start_x + 1;
        int height = draw_end_y - draw_start_y + 1;

        gfx_SetColor(COLOR_ENEMY);
        gfx_FillRectangle(draw_start_x, draw_start_y, width, height);

        gfx_SetColor(255);
        gfx_Rectangle(draw_start_x, draw_start_y, width, height);
    }
}

void raycast_init(void) {
    if (!palette_ready) {
        palette_data[COLOR_BLACK] = gfx_RGBTo1555(0, 0, 0);
        palette_data[COLOR_SKY_NEAR] = gfx_RGBTo1555(90, 120, 200);
        palette_data[COLOR_SKY_MID] = gfx_RGBTo1555(60, 90, 160);
        palette_data[COLOR_SKY_FAR] = gfx_RGBTo1555(30, 50, 110);
        palette_data[COLOR_FLOOR_NEAR] = gfx_RGBTo1555(150, 130, 100);
        palette_data[COLOR_FLOOR_MID] = gfx_RGBTo1555(110, 90, 70);
        palette_data[COLOR_FLOOR_FAR] = gfx_RGBTo1555(70, 60, 50);
        palette_data[COLOR_WALL_LIGHT] = gfx_RGBTo1555(230, 200, 160);
        palette_data[COLOR_WALL_MEDIUM] = gfx_RGBTo1555(190, 150, 110);
        palette_data[COLOR_WALL_DARK] = gfx_RGBTo1555(130, 100, 70);
        palette_data[COLOR_WALL_SHADE_LIGHT] = gfx_RGBTo1555(170, 190, 210);
        palette_data[COLOR_WALL_SHADE_MEDIUM] = gfx_RGBTo1555(120, 150, 180);
        palette_data[COLOR_WALL_SHADE_DARK] = gfx_RGBTo1555(80, 110, 140);
        palette_data[COLOR_DOOR_LIGHT] = gfx_RGBTo1555(200, 170, 70);
        palette_data[COLOR_DOOR_MID] = gfx_RGBTo1555(160, 130, 50);
        palette_data[COLOR_DOOR_DARK] = gfx_RGBTo1555(110, 85, 35);
        palette_data[COLOR_EXIT_LIGHT] = gfx_RGBTo1555(200, 240, 120);
        palette_data[COLOR_EXIT_MID] = gfx_RGBTo1555(160, 210, 90);
        palette_data[COLOR_EXIT_DARK] = gfx_RGBTo1555(100, 160, 50);
        palette_data[COLOR_ENEMY] = gfx_RGBTo1555(230, 60, 60);
        palette_data[COLOR_CROSSHAIR] = gfx_RGBTo1555(255, 200, 80);
        palette_data[COLOR_WEAPON_PRIMARY] = gfx_RGBTo1555(140, 100, 70);
        palette_data[COLOR_WEAPON_ACCENT] = gfx_RGBTo1555(170, 170, 170);

        gfx_SetPalette(palette_data, sizeof(palette_data), 0);
        palette_ready = true;
    }

    float fov_half = FOV / 2.0f;
    float angle_step = FOV / SCREEN_WIDTH;
    for (int x = 0; x < SCREEN_WIDTH; x++) {
        float offset = fov_half - angle_step * (float)x;
        cos_lookup[x] = cosf(offset);
        sin_lookup[x] = sinf(offset);
        depth_template[x] = MAX_DEPTH;
    }
}

