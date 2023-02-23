#include <gb/gb.h>
#include <gbdk/metasprites.h>
#include <stdio.h>

#include "scene.h"

GameCharacter PLAYER;
UINT8 joy, last_joy;
UBYTE is_moving;
INT8 move_x, move_y;

void move_frog() {
    if (move_x != 0) {
        PLAYER.x += (move_x < 0 ? -1 : 1);
        move_metasprite(
            frogger_metasprites[0], 0, 0, PLAYER.x, PLAYER.y);
    } else if (move_y != 0) {
        PLAYER.y += (move_y < 0 ? -1 : 1);
        move_metasprite(
            frogger_metasprites[0], 0, 0, PLAYER.x, PLAYER.y);
    }
}

void main() {
    DISABLE_VBL_TRANSFER;
    OBP1_REG = 0b10011100;
    SPRITES_8x16;  // MUST be 8x16 or 8x8. Can change in different scenes only
    SHOW_BKG;
    SHOW_SPRITES;
    DISPLAY_ON;

    set_sprite_data(0, 4, frogger_tiles);
    set_bkg_data(0, 8, BKG_TILES);
    set_bkg_tiles(0, 0, 20, 18, BKG_MAP);

    PLAYER.x = 56;
    PLAYER.y = 108;
    UINT8 scx_counter = 2;  // REMOVE THIS?
    move_metasprite(
        frogger_metasprites[0], 0, 0, PLAYER.x, PLAYER.y);

    while (1) {
        last_joy = joy;
        joy = joypad();

        if (!is_moving) {
            switch (joy) {
                case J_LEFT:
                    if (CHANGED_BUTTONS & J_LEFT) {
                        is_moving = TRUE;
                        move_x = -12;
                    }
                    break;
                case J_RIGHT:
                    if (CHANGED_BUTTONS & J_RIGHT) {
                        is_moving = TRUE;
                        move_x = 12;
                    }
                    break;
                case J_UP:
                    if (CHANGED_BUTTONS & J_UP) {
                        is_moving = TRUE;
                        move_y = -8;
                    }
                    break;
                case J_DOWN:
                    if (CHANGED_BUTTONS & J_DOWN) {
                        is_moving = TRUE;
                        move_y = 8;
                    }
                    break;
            }
        }
        // --------------------MOVE FROG -------------------------------//

        if (move_x != 0) {
            move_frog();
            move_x += move_x < 0 ? 1 : -1;
        } else if (move_y != 0) {
            move_frog();
            move_y += move_y < 0 ? 1 : -1;
        } else if (move_x == 0 || move_y == 0)
            is_moving = FALSE;

        // --------------------MOVE FROG -------------------------------//

        if (scx_counter == 1) {
            SCX_REG++;
        }
        if (scx_counter == 0)
            scx_counter = 2;
        scx_counter--;

        if (joy & J_SELECT) {
            PLAYER.x = 56;
            PLAYER.y = 108;
            move_metasprite(
                frogger_metasprites[0], 0, 0, PLAYER.x, PLAYER.y);
        }
        wait_vbl_done();
        refresh_OAM();
    }
}