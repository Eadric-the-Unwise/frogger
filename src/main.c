#include <gb/gb.h>
#include <gbdk/metasprites.h>
#include <stdio.h>

#include "scene.h"

GameCharacter PLAYER;
UINT8 joy, last_joy;

void move_frog(INT8 move_x, INT8 move_y) {
    while (move_x != 0) {
        PLAYER.x += (move_x < 0 ? -1 : 1);
        move_metasprite(
            frogger_metasprites[0], 0, 0, PLAYER.x, PLAYER.y);
        move_x += (move_x < 0 ? 1 : -1);
        wait_vbl_done();
        refresh_OAM();
    }
    while (move_y != 0) {
        PLAYER.y += (move_y < 0 ? -1 : 1);
        move_metasprite(
            frogger_metasprites[0], 0, 0, PLAYER.x, PLAYER.y);
        move_y += move_y < 0 ? 1 : -1;
        wait_vbl_done();
        refresh_OAM();
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

    PLAYER.x = 72;
    PLAYER.y = 88;
    move_metasprite(
        frogger_metasprites[0], 0, 0, PLAYER.x, PLAYER.y);

    while (1) {
        last_joy = joy;
        joy = joypad();

        switch (joy) {
            case J_LEFT:
                if (CHANGED_BUTTONS & J_LEFT) {
                    move_frog(-8, 0);
                }
                break;
            case J_RIGHT:
                if (CHANGED_BUTTONS & J_RIGHT) {
                    move_frog(8, 0);
                }
                break;
            case J_UP:
                if (CHANGED_BUTTONS & J_UP) {
                    move_frog(0, -8);
                }
                break;
            case J_DOWN:
                if (CHANGED_BUTTONS & J_DOWN) {
                    move_frog(0, 8);
                }
                break;
        }

        wait_vbl_done();
        refresh_OAM();
    }
}