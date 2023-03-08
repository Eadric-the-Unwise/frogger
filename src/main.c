#include <gb/gb.h>
#include <gbdk/metasprites.h>
#include <stdio.h>

#include "scene.h"
//------------GOALS-------------//
// ANIMATE TURTLES
// WIN SCREEN
// SCORE BOARD
// 1 UP SYSTEM
// GAME OVER
// CGB PALLETES
// SOUND
// STAGE 2
GameCharacter PLAYER;
UINT8 joy, last_joy;
UBYTE is_moving, turtles_diving;
INT8 move_x, move_y;
UINT8 scx_counter, turtle_counter, turtle_dive_counter;
// UBYTE on_turtle, on_log3;
UINT8 turtle_tiles[4] = {0x10, 0x11, 0x12, 0x12};
UINT8 dive_tiles[7] = {0x22, 0x23, 0x02, 0x02, 0x23, 0x22, NULL};
UINT8 turtle_tile_index, dive_tile_index;
// UINT8 *turtle_tile_ptr = turtle_tiles;

void reset_frog() {
    is_moving = FALSE;
    move_x = move_y = 0;
    PLAYER.x = 56;   // 56
    PLAYER.y = 108;  // 108
    PLAYER.position = ON_NOTHING;
    move_metasprite(
        frogger_metasprites[0], 0, 0, PLAYER.x, PLAYER.y);
}
void init_level() {
    set_sprite_data(0, 4, frogger_tiles);
    set_bkg_data(0, 36, BKG_TILES);
    set_bkg_tiles(0, 0, 32, 32, BKG_MAP);
    turtles_diving = FALSE;

    reset_frog();
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
void scroll_counters() {
    if (scx_counter % 6 == 0) {
        SCROLL_LOG1 -= 1;     // LOG 1
        SCROLL_TURTLE1 += 1;  // TURTLES 1
        SCROLL_TURTLE2 += 1;  // TURTLES 2

        if (PLAYER.position == ON_TURTLE) {
            PLAYER.x -= 1;
            move_metasprite(
                frogger_metasprites[0], 0, 0, PLAYER.x, PLAYER.y);
        }
        if (PLAYER.position == ON_LOG1) {
            PLAYER.x += 1;
            move_metasprite(
                frogger_metasprites[0], 0, 0, PLAYER.x, PLAYER.y);
        }
    }
    if (scx_counter % 5 == 0) {
        SCROLL_LOG3 -= 1;  // LOG 3
        if (PLAYER.position == ON_LOG3) {
            PLAYER.x += 1;
            move_metasprite(
                frogger_metasprites[0], 0, 0, PLAYER.x, PLAYER.y);
        }
    }
    if (scx_counter % 4 == 0) {  // += moves the 'camera' to the right, resulting in objects on screen moving left
        SCROLL_LOG2 -= 1;        // LOG 2
        SCROLL_CAR3 += 1;        // CAR3
        SCROLL_CAR4 -= 1;        // CAR4
        SCROLL_CAR5 += 1;        // CAR5
        if (PLAYER.position == ON_LOG2) {
            PLAYER.x += 1;
            move_metasprite(
                frogger_metasprites[0], 0, 0, PLAYER.x, PLAYER.y);
        }
    }

    if (scx_counter % 3 == 0) {
        SCROLL_CAR1 += 1;  // CAR 1 BUS
    }
    SCROLL_CAR2 -= 1;  // CAR 2 PINK

    scx_counter++;
}
void win_check(UINT8 frogx, UINT8 frogy) {
    UINT16 left_x, right_x, indexY, tileindex_L, tileindex_R;
    UINT8 left_offset, right_offset;

    left_offset = WATER_OFFSET_L;
    right_offset = WATER_OFFSET_R;

    indexY = (frogy + 8) / 8;              // CALCULATE THE FROG'S Y ROW, USING HIS CENTER Y
    left_x = (frogx + left_offset) / 8;    // CALCULATE THE LEFT OF FROG X
    right_x = (frogx + right_offset) / 8;  // CALCULATE THE RIGHT OF FROG X

    tileindex_L = 32 * indexY + left_x;
    tileindex_R = 32 * indexY + right_x;

    if (COLLISION_MAP[tileindex_L] == 0x01 && COLLISION_MAP[tileindex_R] == 0x01) {
        set_bkg_tiles(1, 1, 2, 2, WIN_FROG);
        reset_frog();
    } else if (COLLISION_MAP[tileindex_L] == 0x02 && COLLISION_MAP[tileindex_R] == 0x02) {
        set_bkg_tiles(5, 1, 2, 2, WIN_FROG);
        reset_frog();
    } else if (COLLISION_MAP[tileindex_L] == 0x03 && COLLISION_MAP[tileindex_R] == 0x03) {
        set_bkg_tiles(9, 1, 2, 2, WIN_FROG);
        reset_frog();
    } else if (COLLISION_MAP[tileindex_L] == 0x04 && COLLISION_MAP[tileindex_R] == 0x04) {
        set_bkg_tiles(13, 1, 2, 2, WIN_FROG);
        reset_frog();
    } else if (COLLISION_MAP[tileindex_L] == 0x05 && COLLISION_MAP[tileindex_R] == 0x05) {
        set_bkg_tiles(17, 1, 2, 2, WIN_FROG);
        reset_frog();
    } else
        reset_frog();
}
void collide_check(UINT8 frogx, UINT8 frogy) {
    UINT16 left_x, right_x, indexY;
    UINT8 left_offset, right_offset;
    if (PLAYER.y >= STREET) {
        left_offset = STREET_OFFSET_L;
        right_offset = STREET_OFFSET_R;
    } else {
        left_offset = WATER_OFFSET_L;
        right_offset = WATER_OFFSET_R;
    }

    indexY = (frogy + 8) / 8;                              // CALCULATE THE FROG'S Y ROW, USING HIS CENTER Y
    UINT8 *scroll_ptr = scroll_remap[indexY];              // POINT TO THE CURRENT UINT8 SCROLL X VALUE
    UINT8 vblank_offset = scroll_ptr ? *scroll_ptr : 0;    // IF POINTING TO NULL, THEN OFFSET IS 0. OTHERWISE, OFFSET EQUALS THE CURRENT SCROLL X VALUE
    left_x = (frogx + left_offset + vblank_offset) / 8;    // CALCULATE THE LEFT OF FROG X
    right_x = (frogx + right_offset + vblank_offset) / 8;  // CALCULATE THE RIGHT OF FROG X

    // 1024 total VRAM tiles 32x32
    UINT16 left_tile = get_bkg_tile_xy(left_x & 31, indexY);    // '& 31' WRAPPER PREVENTS VALUE FROM GOING HIGHER THAN 31. IT LOOPS BACK TO 0 (binary AND)
    UINT16 right_tile = get_bkg_tile_xy(right_x & 31, indexY);  // '& 31' WRAPPER PREVENTS VALUE FROM GOING HIGHER THAN 31. IT LOOPS BACK TO 0 (binary AND)

    //? PLAYER.y % 8 ==4

    if (PLAYER.y >= STREET) {                                                                                                                  // ON OR BELOW THE SIDEWALK
        if ((left_tile >= CAR_TILES_START && left_tile <= CAR_TILES_END) || (right_tile >= CAR_TILES_START && right_tile <= CAR_TILES_END)) {  // CHECK FOR ALL CAR TILE COLLISIONS
            reset_frog();
        }
    } else if (PLAYER.y == TURTLE1 || PLAYER.y == TURTLE2) {
        if ((left_tile >= TURTLE_TILES_START && left_tile <= TURTLE_TILES_END || left_tile >= TURTLE_TILES_END && right_tile <= TURTLE_TILES_END)) {  // CHECK ALL TURTLE TILES
            PLAYER.position = ON_TURTLE;                                                                                                              // MOVES FROG AT TURTLE SPEED IN scroll_counters();
        } else {
            if (!is_moving)  // KILL FROG ONLY AFTER IT COMPLETES ITS MOVEMENT
                reset_frog();
        }
    } else if (PLAYER.y == LOG1) {
        if ((left_tile >= LOG_TILES_START && left_tile <= LOG_TILES_END || left_tile >= LOG_TILES_START && right_tile <= LOG_TILES_END)) {  // CHECK ALL LOG TILES
            PLAYER.position = ON_LOG1;                                                                                                      // MOVES FROG AT LOG SPEED IN scroll_counters();
        } else {
            if (!is_moving)  // KILL FROG ONLY AFTER IT COMPLETES ITS MOVEMENT
                reset_frog();
        }
    } else if (PLAYER.y == LOG2) {
        if ((left_tile >= LOG_TILES_START && left_tile <= LOG_TILES_END || left_tile >= LOG_TILES_START && right_tile <= LOG_TILES_END)) {  // CHECK ALL LOG TILES
            PLAYER.position = ON_LOG2;                                                                                                      // MOVES FROG AT LOG SPEED IN scroll_counters();
        } else {
            if (!is_moving)  // KILL FROG ONLY AFTER IT COMPLETES ITS MOVEMENT
                reset_frog();
        }
    } else if (PLAYER.y == LOG3) {
        if ((left_tile >= LOG_TILES_START && left_tile <= LOG_TILES_END || left_tile >= LOG_TILES_START && right_tile <= LOG_TILES_END)) {  // CHECK ALL LOG TILES
            PLAYER.position = ON_LOG3;                                                                                                      // MOVES FROG AT LOG SPEED IN scroll_counters();
        } else {
            if (!is_moving)  // KILL FROG ONLY AFTER IT COMPLETES ITS MOVEMENT
                reset_frog();
        }
    } else if (PLAYER.y == WIN) {
        win_check(PLAYER.x, PLAYER.y);
    }
}
void animate_turtles() {
    turtle_counter++;
    turtle_dive_counter++;
    // turtle_dive_counter++;
    UINT8 row1, row2;
    UINT8 frame = NULL;
    if (turtle_counter % 16 == 0) {
        frame = turtle_tiles[turtle_tile_index++ % 4];  // TURTLE TILE FRAME OF ANIMATION (1 % 4 = 1) ++ modifies the turtle_tile_index variable each loop
        for (UINT8 i = 0; i < 32; i++) {
            row1 = get_bkg_tile_xy(i, 4);  // TURTLES1 ROW
            row2 = get_bkg_tile_xy(i, 7);  // TURTLES2 ROW
            if (row1 >= TURTLE_TILES_START && row1 <= TURTLE_TILES_END) {
                set_bkg_tile_xy(i, 4, frame);  // TURTLES1 ROW
            }
            if (row2 >= TURTLE_TILES_START && row2 <= TURTLE_TILES_END) {
                set_bkg_tile_xy(i, 7, frame);  // TURTLES2 ROW
            }
        }
    }
    if ((turtle_dive_counter == 48) && (!turtles_diving)) {
        turtles_diving = TRUE;
    }
    if (turtles_diving) {
        if (turtle_counter % 16 == 0) {
            UINT8 dive_frame = dive_tiles[dive_tile_index++ % 7];  // TURTLE TILE FRAME OF ANIMATION (1 % 4 = 1) ++ modifies the turtle_tile_index variable each loop

            if (dive_frame == NULL) {
                set_bkg_tile_xy(0x0A, 4, frame);
                set_bkg_tile_xy(0x0B, 4, frame);
                turtles_diving = FALSE;
                turtle_dive_counter = 0;
            } else {
                set_bkg_tile_xy(0x0A, 4, dive_frame);
                set_bkg_tile_xy(0x0B, 4, dive_frame);
            }
        }
    }
}
// void animate_dive_turtles() {
//     if (turtle_counter % 16 == 0) {
//         UINT8 frame = dive_tiles[dive_tile_index++ % 6];  // TURTLE TILE FRAME OF ANIMATION (1 % 4 = 1) ++ modifies the turtle_tile_index variable each loop
//         set_bkg_tile_xy(0x0A, 4, frame);
//     }
// }

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

    init_level();

    while (1) {
        last_joy = joy;
        joy = joypad();
        // -------------------- COLLISION CHECK -------------------------------//
        collide_check(PLAYER.x, PLAYER.y);

        // -------------------- BUTTON INPUT -------------------------------//
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
                        PLAYER.position = ON_NOTHING;
                    }
                    break;
                case J_DOWN:
                    if ((PLAYER.y <= 100) && (CHANGED_BUTTONS & J_DOWN)) {  // PREVENT FROG FROM GOING BELOW SPAWN POINT
                        is_moving = TRUE;
                        move_y = 8;
                        PLAYER.position = ON_NOTHING;
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

        // ---------------- SCROLL COUNTERS --------------------------- //
        scroll_counters();
        // ---------------- ANIMATE TURTLES --------------------------- //
        // if (turtles_diving) {
        //     animate_dive_turtles();
        // }
        animate_turtles();
        // -------------------- DEBUG -------------------------------//
        if (joy & J_SELECT) {
            reset_frog();
        }
        if (joy & J_A) {
            // printf("%u\n", PLAYER.y);
            set_bkg_tile_xy(4, 4, 0x11);
        }
        wait_vbl_done();
        refresh_OAM();
    }
}