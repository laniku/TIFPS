#include "ui.h"
#include "player.h"
#include "raycast.h"
#include <graphx.h>

#define BAR_WIDTH 100
#define BAR_HEIGHT 8
#define BAR_X 10
#define HP_BAR_Y 10
#define AMMO_BAR_Y 25
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

#define WEAPON_WIDTH 40
#define WEAPON_HEIGHT 60
#define WEAPON_X ((SCREEN_WIDTH - WEAPON_WIDTH) / 2)
#define WEAPON_Y (SCREEN_HEIGHT - WEAPON_HEIGHT - 10)

#define CROSSHAIR_SIZE 8
#define CROSSHAIR_X (SCREEN_WIDTH / 2)
#define CROSSHAIR_Y (SCREEN_HEIGHT / 2)
static bool muzzle_flash = false;

void ui_init(void) {
}

void ui_render(void) {
    int hp = player_get_hp();
    int ammo = player_get_ammo();
    int max_hp = 100;
    int max_ammo = 50;

    gfx_SetColor(255);
    gfx_Rectangle(BAR_X, HP_BAR_Y, BAR_WIDTH, BAR_HEIGHT);
    int hp_width = (hp * (BAR_WIDTH - 2)) / max_hp;
    if (hp_width > 0) {
        gfx_SetColor(RAYCAST_COLOR_HP_FILL);
        gfx_FillRectangle(BAR_X + 1, HP_BAR_Y + 1, hp_width, BAR_HEIGHT - 2);
    }

    gfx_SetColor(255);
    gfx_Rectangle(BAR_X, AMMO_BAR_Y, BAR_WIDTH, BAR_HEIGHT);
    int ammo_width = (ammo * (BAR_WIDTH - 2)) / max_ammo;
    if (ammo_width > 0) {
        gfx_SetColor(RAYCAST_COLOR_AMMO_FILL);
        gfx_FillRectangle(BAR_X + 1, AMMO_BAR_Y + 1, ammo_width, BAR_HEIGHT - 2);
    }

    gfx_SetColor(RAYCAST_COLOR_WEAPON_PRIMARY);
    gfx_FillRectangle(WEAPON_X + 15, WEAPON_Y + 20, 10, 30);
    gfx_FillRectangle(WEAPON_X + 18, WEAPON_Y + 45, 4, 12);
    gfx_SetColor(RAYCAST_COLOR_WEAPON_ACCENT);
    gfx_FillRectangle(WEAPON_X + 17, WEAPON_Y + 10, 6, 15);

    gfx_SetColor(255);
    gfx_Rectangle(WEAPON_X + 15, WEAPON_Y + 20, 10, 30);
    gfx_Rectangle(WEAPON_X + 17, WEAPON_Y + 10, 6, 15);
    gfx_Rectangle(WEAPON_X + 18, WEAPON_Y + 45, 4, 12);
    gfx_Line(WEAPON_X + 17, WEAPON_Y + 25, WEAPON_X + 17, WEAPON_Y + 20);
    gfx_Line(WEAPON_X + 23, WEAPON_Y + 25, WEAPON_X + 23, WEAPON_Y + 20);

    if (muzzle_flash || player_is_shooting()) {
        gfx_SetColor(255);
        gfx_Rectangle(WEAPON_X + 16, WEAPON_Y + 5, 8, 8);
        gfx_Line(WEAPON_X + 16, WEAPON_Y + 9, WEAPON_X + 24, WEAPON_Y + 9);
        gfx_Line(WEAPON_X + 20, WEAPON_Y + 5, WEAPON_X + 20, WEAPON_Y + 13);
    }
}

void ui_render_crosshair(void) {
    gfx_SetColor(RAYCAST_COLOR_CROSSHAIR);
    gfx_Line(CROSSHAIR_X - CROSSHAIR_SIZE, CROSSHAIR_Y,
             CROSSHAIR_X + CROSSHAIR_SIZE, CROSSHAIR_Y);
    gfx_Line(CROSSHAIR_X, CROSSHAIR_Y - CROSSHAIR_SIZE,
             CROSSHAIR_X, CROSSHAIR_Y + CROSSHAIR_SIZE);
    gfx_SetPixel(CROSSHAIR_X, CROSSHAIR_Y);
}

void ui_set_muzzle_flash(bool active) {
    muzzle_flash = active;
}

