#ifndef UI_H
#define UI_H

#include <stdbool.h>

void ui_init(void);
void ui_render(void);
void ui_render_crosshair(void);
void ui_set_muzzle_flash(bool active);

#endif
