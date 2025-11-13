#ifndef RAYCAST_H
#define RAYCAST_H

#define RAYCAST_COLOR_HP_FILL 16
#define RAYCAST_COLOR_AMMO_FILL 10
#define RAYCAST_COLOR_ENEMY 19
#define RAYCAST_COLOR_CROSSHAIR 20
#define RAYCAST_COLOR_WEAPON_PRIMARY 21
#define RAYCAST_COLOR_WEAPON_ACCENT 22

void raycast_render(void);
void raycast_init(void);
void raycast_render_enemies(void);

#endif
