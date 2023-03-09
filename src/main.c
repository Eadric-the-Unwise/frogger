#include <gb/gb.h>
#include <gbdk/metasprites.h>
#include <stdio.h>
#include <string.h>

#include "scene.h"
//------------GOALS-------------//
// SCORE BOARD
// TIMER + SCORE
// WIN SCREEN
// 1 UP SYSTEM
// GAME OVER
// CGB PALLETES
// SOUND
// STAGE 2
GameCharacter PLAYER;
UINT8 joy, last_joy;
UBYTE is_moving, turtles_diving;
INT8 move_x, move_y;
UINT8 scx_counter, turtle_counter, dive_counter;

UINT8 turtle_divers1[] = {0x0A, 0x0B, NULL};        // ROW 1 TURTLES
UINT8 turtle_divers2[] = {0x10, 0x11, 0x12, NULL};  // ROW 2 TURTLES

UINT8 turtle_tiles[] = {TURTLE_TILES_START, 0x11, TURTLE_TILES_END, TURTLE_TILES_END};                                    // REGULAR TURTLE ANIMATION LOOP
UINT8 dive_tiles[] = {DIVE_TILES_START, DIVE_TILES_END, WATER_TILE, WATER_TILE, DIVE_TILES_END, DIVE_TILES_START, NULL};  // DIVING TURTLE ANIMATION
UINT8 turtle_tile_index, dive_tile_index;

uint8_t *vram_addr;

// UINT32 score = 87654321; //USE THIS IF HIGHER THAN 65535
UINT16 score;
UINT8 text_buffer[16];  // if score = 150, text_buffer[0] = '1',text_buffer[1] = '5', text_buffer[2] = '0', text_buffer[3] = 0

// void render_32bit_score(UINT32 score, UINT8 *buffer) {  // THIS FUNCTION IS A CUSTOM SPRINTF BECAUSE SPRINTF ONLY SUPPORTS UP TO 16 BIT SON GB. THIS FUNC ALLOWS US TO USE 32BIT AND MANUALLY MANIPULATE THE ARRAY
//     UINT8 *old_buffer = buffer;                         // buffer = text_buffer[0]
//     while (score > 0) {
//         // dereference buffer[0] (*), change its value, THEN ++ to buffer[1] (++buffer vs buffer++) pre-increment vs post-increment
//         // score % 10 gets me the farthest right digit, then dividing by 10 removes that right digit, which brings me to the next digit to its left
//         // By adding '0', we convert the integer into an ASCII code value. So: 5 + '0' = '5'
//         *buffer++ = (score % 10) + '0';
//         score /= 10;  // shifts out the rightmost digit
//     }
//     // REMEMBER bugger is actually text_buffer[?] because of previous buffer++! Hence why we need old_buffer
//     *buffer = 0;  // when the while loop runs out of digits to add to the buffer, change the next character in the array to equal 0. This is check inside of update_score()'s for loop.
//     reverse(old_buffer);  // reverses the pointer's contents until it reaches a 0 or NULL in the array, so that the score displays correctly
// }

void update_score() {
    UINT8 vram_offset = 96;
    vram_addr = get_bkg_xy_addr(0, 15);

    // if (score > 65535) {
    //     render_32bit_score(score, text_buffer);
    // } else
    // sprintf maxes at 16 bits on GAMEBOY
    sprintf(text_buffer, "%u", score);  // stores the ascii code value into a buffer, one value per text_buffer[] item (SEE text_buffer[16] above)

    for (UINT8 *ptr = text_buffer; *ptr != 0; ptr++) {  // when for loop hits a 0 (NOT A '0', WHICH IS ASCII), the loop ends
        set_vram_byte(vram_addr, *ptr + vram_offset);
        vram_addr++;
    }
}

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
    set_bkg_data(0x80, 68, FONT);
    set_bkg_tiles(0, 0, 32, 32, BKG_MAP);
    turtles_diving = FALSE;
    score = 0;
    update_score();

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
        if ((left_tile >= TURTLE_TILES_START && left_tile <= DIVE_TILES_END || left_tile >= TURTLE_TILES_START && right_tile <= DIVE_TILES_END)) {  // CHECK ALL TURTLE TILES
            PLAYER.position = ON_TURTLE;                                                                                                            // MOVES FROG AT TURTLE SPEED IN scroll_counters();
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
    turtle_counter++;  // REGULAR TURTLES ANIMATION TIMER
    dive_counter++;    // DIVING TURTLES ANIMATION TIMER
    UINT8 row1, row2;
    UINT8 regular_frame = NULL;  // REGULAR TURTLE ANIMATION LOOP FRAMES

    // -------------------------- REGULAR TURTLES -------------------------- //
    if (turtle_counter % 16 == 0) {                             // ANIMATE ALL TURTLES FRAME EVERY 16 GAME LOOPS
        regular_frame = turtle_tiles[turtle_tile_index++ % 4];  // TURTLE TILE FRAME OF ANIMATION (1 % 4 = 1) ++ modifies the turtle_tile_index variable each loop
        for (UINT8 i = 0; i < 32; i++) {
            row1 = get_bkg_tile_xy(i, 4);  // TURTLES1 ROW
            row2 = get_bkg_tile_xy(i, 7);  // TURTLES2 ROW
            if (row1 >= TURTLE_TILES_START && row1 <= TURTLE_TILES_END) {
                set_bkg_tile_xy(i, 4, regular_frame);  // TURTLES1 ROW
            }
            if (row2 >= TURTLE_TILES_START && row2 <= TURTLE_TILES_END) {
                set_bkg_tile_xy(i, 7, regular_frame);  // TURTLES2 ROW
            }
        }
    }

    // -------------------------- DIVING TURTLES -------------------------- //
    if ((dive_counter == 48) && (!turtles_diving)) {  // BEGIN DIVING TURTLES ANIMATIONS AFTER 48 GAME LOOPS
        turtles_diving = TRUE;                        // SEE BELOW
    }
    if (turtles_diving) {  // BEGIN DIVING ANIMATIONS
        if (turtle_counter % 16 == 0) {
            UINT8 diving_frame = dive_tiles[dive_tile_index++ % 7];  // TURTLE TILE FRAME OF ANIMATION (1 % 4 = 1) ++ modifies the turtle_tile_index variable each loop

            if (diving_frame == NULL) {  // IF THE DIVER ANIMATION IS COMPLETED
                for (UINT8 *ptr = turtle_divers1; *ptr != NULL; ptr++) {
                    set_bkg_tile_xy(*ptr, 4, regular_frame);  // RETURN ALL ROW 1 DIVING TURTLES TO ALL REGULAR TURTLES' ANIMATION FRAME
                }
                for (UINT8 *ptr = turtle_divers2; *ptr != NULL; ptr++) {
                    set_bkg_tile_xy(*ptr, 7, regular_frame);  // RETURN ALL ROW 2 DIVING TURTLES TO ALL REGULAR TURTLES' ANIMATION FRAME
                }

                turtles_diving = FALSE;  // END DIVING ANIMATION
                dive_counter = 0;        // RESET DIVE COUNTER
            } else {                     // ANIMATE DIVE ANIMATIONS
                for (UINT8 *ptr = turtle_divers1; *ptr != NULL; ptr++) {
                    set_bkg_tile_xy(*ptr, 4, diving_frame);  // ROW 1 TURTLES
                }
                for (UINT8 *ptr = turtle_divers2; *ptr != NULL; ptr++) {
                    set_bkg_tile_xy(*ptr, 7, diving_frame);  // ROW 2 TURTLES
                }
            }
        }
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