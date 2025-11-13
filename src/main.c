#include <stdint.h>
#include <stdbool.h>
#include <graphx.h>
#include <keypadc.h>
#include <fileioc.h>
#include "game.h"
#include "raycast.h"
#include "player.h"
#include "level.h"
#include "enemy.h"
#include "ui.h"

#define FRAME_SKIP 2
#define RENDER_SKIP 1

int main(void) {
    gfx_Begin();
    gfx_SetDrawBuffer();

    game_init();

    static int frame_counter = 0;

    while (game_is_running()) {
        kb_Scan();

        if ((frame_counter % (FRAME_SKIP + 1)) == 0) {
            game_update();
        }

        if ((frame_counter % (RENDER_SKIP + 1)) == 0) {
            game_render();
            gfx_SwapDraw();
        }

        frame_counter++;
    }

    game_cleanup();
    gfx_End();

    return 0;
}

