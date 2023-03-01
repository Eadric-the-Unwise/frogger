#include <gb/gb.h>
#include <gbdk/metasprites.h>
#include <stdio.h>

#include "scene.h"
//------------GOALS-------------//
// LOG COLLISIONS
// TURTLE COLLISIONS
// CGB PALLETES
// SOUND
GameCharacter PLAYER;
UINT8 joy, last_joy;
UBYTE is_moving;
INT8 move_x, move_y;
UINT8 scx_counter;

void respawn_frog() {
    is_moving = FALSE;
    move_x = move_y = 0;
    PLAYER.x = 56;
    PLAYER.y = 108;
    move_metasprite(
        frogger_metasprites[0], 0, 0, PLAYER.x, PLAYER.y);
}

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

void parallaxScroll() {  // CAMERA Y POSITION IN TILE ROWS
    switch (LYC_REG) {
        case 0x00:  // first three stationary rows
            move_bkg(0, 0);
            LYC_REG = 0x17;
            break;
        case 0x17:  // LOG 1
            move_bkg(SCROLL_LOG1, 0);
            LYC_REG = 0x1F;
            break;
        case 0x1F:  // TURLES 1
            move_bkg(SCROLL_TURTLE1, 0);
            LYC_REG = 0x27;
            break;
        case 0x27:  // LOG 2 (fast)
            move_bkg(SCROLL_LOG2, 0);
            LYC_REG = 0x2F;
            break;
        case 0x2F:  // LOG 3
            move_bkg(SCROLL_LOG3, 0);
            LYC_REG = 0x37;
            break;
        case 0x37:  // TURTLES 2
            move_bkg(SCROLL_TURTLE2, 0);
            LYC_REG = 0x40;
            break;
        case 0x40:  // MIDDLE WALL
            move_bkg(0, 0);
            LYC_REG = 0x48;
            break;
        case 0x48:  // CAR 1 BUS
            move_bkg(SCROLL_CAR1, 0);
            LYC_REG = 0x50;
            break;
        case 0x50:  // CAR 2 PINK
            move_bkg(SCROLL_CAR2, 0);
            LYC_REG = 0x58;
            break;
        case 0x58:  // CAR 3
            move_bkg(SCROLL_CAR3, 0);
            LYC_REG = 0x60;
            break;
        case 0x60:  // CAR 4
            move_bkg(SCROLL_CAR4, 0);
            LYC_REG = 0x68;
            break;
        case 0x68:  // CAR 5
            move_bkg(SCROLL_CAR5, 0);
            LYC_REG = 0x6F;
            break;
        case 0x6F:  // BOTTOM CURB AND BELOW, STATIONARY
            move_bkg(0, 0);
            LYC_REG = 0x00;
            break;
    }
}

void collide_check(UINT8 frogx, UINT8 frogy) {
    UINT16 left_x, right_x, indexY;

    indexY = (frogy + 8) / 8;                     // CALCULATE THE FROG'S Y ROW, USING HIS CENTER Y
    UINT8 *scroll_ptr = scroll_remap[indexY];     // POINT TO THE CURRENT UINT8 SCROLL X VALUE
    UINT8 offset = scroll_ptr ? *scroll_ptr : 0;  // IF POINTING TO NULL, THEN OFFSET IS 0. OTHERWISE, OFFSET EQUALS THE CURRENT SCROLL X VALUE
    left_x = (frogx + offset) / 8;                // CALCULATE THE LEFT OF FROG X
    right_x = (frogx + 15 + offset) / 8;          // CALCULATE THE RIGHT OF FROG X

    // 1024 total VRAM tiles 32x32
    UINT16 left_tile = get_bkg_tile_xy(left_x & 31, indexY);    // '& 31' WRAPPER PREVENTS VALUE FROM GOING HIGHER THAN 31. IT LOOPS BACK TO 0 (binary AND)
    UINT16 right_tile = get_bkg_tile_xy(right_x & 31, indexY);  // '& 31' WRAPPER PREVENTS VALUE FROM GOING HIGHER THAN 31. IT LOOPS BACK TO 0 (binary AND)

    if ((left_tile >= 0x05 && left_tile <= 0x0F) || (right_tile >= 0x05 && right_tile <= 0x0F)) {  // CHECK FOR ALL CAR TILE COLLISIONS
        respawn_frog();
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

        collide_check(PLAYER.x, PLAYER.y);

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
                    if ((PLAYER.y <= 100) && (CHANGED_BUTTONS & J_DOWN)) {  // PREVENT FROG FROM GOING BELOW SPAWN POINT
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

        if (scx_counter % 6 == 0) {
            SCROLL_LOG1 -= 1;     // LOG 1
            SCROLL_TURTLE1 += 1;  // TURTLES 1
            SCROLL_TURTLE2 += 1;  // TURTLES 2
        }
        if (scx_counter % 5 == 0) {
            SCROLL_LOG3 -= 1;  // LOG 3
        }
        if (scx_counter % 4 == 0) {  // += moves the 'camera' to the right, resulting in objects on screen moving left
            SCROLL_LOG2 -= 1;        // LOG 2
            SCROLL_CAR3 += 1;        // CAR3
            SCROLL_CAR4 -= 1;        // CAR4
            SCROLL_CAR5 += 1;        // CAR5
        }

        if (scx_counter % 3 == 0) {
            SCROLL_CAR1 += 1;  // CAR 1 BUS
        }
        SCROLL_CAR2 -= 1;  // CAR 2 PINK

        scx_counter++;

        if (joy & J_SELECT) {
            respawn_frog();
        }
        wait_vbl_done();
        refresh_OAM();
    }
}