#include <gb/gb.h>
#include <gbdk/metasprites.h>
#include <stdio.h>

#include "scene.h"
//------------GOALS-------------//
// SCREEN SCROLL + LYC_REG EDIT
// CGB PALLETES
// SOUND
GameCharacter PLAYER;
UINT8 joy, last_joy;
UBYTE is_moving;
INT8 move_x, move_y;
UINT8 scroll1, scroll2, scroll3, scroll4, scroll5, scroll6, scroll7, scroll8;
UINT8 scx_counter;

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

void parallaxScroll() {
    switch (LYC_REG) {
        case 0x00:
            move_bkg(0, 0);
            LYC_REG = 0x17;  // 56px
            break;
        case 0x17:
            move_bkg(scroll1, 0);
            LYC_REG = 0x1F;  // 56px
            break;
        case 0x1F:
            move_bkg(scroll2, 0);
            LYC_REG = 0x40;  // 56px
            break;
        case 0x40:
            move_bkg(0, 0);
            LYC_REG = 0x48;
            break;
        case 0x48:
            move_bkg(scroll3, 0);
            LYC_REG = 0x50;
            break;
        case 0x50:
            move_bkg(scroll4, 0);
            LYC_REG = 0x58;
            break;
        case 0x58:
            move_bkg(scroll5, 0);
            LYC_REG = 0x60;
            break;
        case 0x60:
            move_bkg(scroll6, 0);
            LYC_REG = 0x68;
            break;
        case 0x68:
            move_bkg(scroll7, 0);
            LYC_REG = 0x6F;
            break;
        case 0x6F:
            move_bkg(scroll8, 0);
            LYC_REG = 0x00;
            break;
    }
}

void main() {
    STAT_REG = 0x45;  // enable LYC=LY interrupt so that we can set a specific line it will fire at //
    LYC_REG = 0x00;

    CRITICAL {
        add_LCD(parallaxScroll);
    }
    set_interrupts(VBL_IFLAG | LCD_IFLAG);

    DISABLE_VBL_TRANSFER;
    OBP1_REG = 0b10011100;
    SPRITES_8x16;  // MUST be 8x16 or 8x8. Can change in different scenes only
    SHOW_BKG;
    SHOW_SPRITES;
    DISPLAY_ON;

    set_sprite_data(0, 4, frogger_tiles);
    set_bkg_data(0, 17, BKG_TILES);
    set_bkg_tiles(0, 0, 32, 32, BKG_MAP);

    PLAYER.x = 56;
    PLAYER.y = 108;

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

        if (scx_counter % 4 == 0) {  // += moves the 'camera' to the right, resulting in objects on screen moving left
            scroll1 -= 1;            // WATER
            scroll2 += 1;
            scroll5 += 1;  // CAR3
            scroll6 -= 1;  // CAR4
            scroll7 += 1;  // CAR5
        }
        if (scx_counter % 3 == 0) {
            scroll3 += 1;  // BUS
        }
        scroll4 -= 1;  // PINK CAR

        scx_counter++;

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