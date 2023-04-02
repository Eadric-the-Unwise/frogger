#include <gb/gb.h>
#include <gbdk/metasprites.h>
#include <stdio.h>
#include <string.h>

#include "scene.h"
//------------GOALS-------------//
// debugger
// WIN SCREEN
// GAME OVER
// CGB PALLETES
// SOUND
// STAGE 2

// pc to laptop testing //
// laptop to pc testing //
GameCharacter PLAYER; // FROG
UINT8 joy, last_joy;
UBYTE is_moving, turtles_diving, is_animating; // IS MOVING = LOCKS JOY WHILE FROG IS ANIMATION TO NEXT TILE // TURTLES DIVING = TURTLES CURRENTLY ANIMATING DIVE ANIMATION
INT8 move_x, move_y;                           // IF NOT 0, MOVE FROG 1 PIXEL PER LOOP
UINT8 scx_counter;                             // VBLANK INTERRUPT COUNTER
UINT8 turtle_counter, dive_counter;            // TURTLE ANIMATION COUNTER
UINT16 timer;
UINT8 timer_tick; // STAGE TIMER
UBYTE GAMESTATE;

UINT8 turtle_divers1[] = {0x0A, 0x0B, NULL};       // ROW 1 TURTLES
UINT8 turtle_divers2[] = {0x10, 0x11, 0x12, NULL}; // ROW 2 TURTLES

UINT8 turtle_tiles[] = {TURTLE_TILES_START, 0x11, TURTLE_TILES_END, TURTLE_TILES_END};                                   // REGULAR TURTLE ANIMATION LOOP
UINT8 dive_tiles[] = {DIVE_TILES_START, DIVE_TILES_END, WATER_TILE, WATER_TILE, DIVE_TILES_END, DIVE_TILES_START, NULL}; // DIVING TURTLE ANIMATION
UINT8 turtle_tile_index, dive_tile_index;

UINT8 y_min;

uint8_t *vram_addr;

UINT8 lives = 4;
// UINT32 score = 87654321; //USE THIS IF HIGHER THAN 65535
UINT16 score;
UINT8 score_buffer[5]; // MAX SCORE 9999 = {'9', '9', '9', '9', 0}; // if score = 150, score_buffer[0] = '1',score_buffer[1] = '5', score_buffer[2] = '0', score_buffer[3] = 0

UINT8 PLAYER1UP_text[] = "1 UP";
UINT8 PLAYER2UP_text[] = "2 UP";

UINT8 level_text[] = "LEVEL ";
UINT8 current_level;
UINT16 level_buffer[12];

// DISPLAY LEVEL 1, LEVEL 2, LEVEL 3 AT BOTTOM
UINT8 animation_timer = 1;
UINT8 animation_phase;

void render_animations()
{

    if (animation_timer % 2 == 0)
    {
        // move_metasprite(frogger_up_down_animation[animation_phase], 0, 0, PLAYER.x, PLAYER.y);
        animation_phase++;
        if (frogger_up_down_animation[animation_phase] == NULL)
        {
            if (PLAYER.direction == DOWN)
                move_metasprite_hflip(
                    frogger_metasprites[0], 0, 0, PLAYER.x, PLAYER.y + 48);
            else if (PLAYER.direction == UP)
                move_metasprite(frogger_metasprites[0], 0, 0, PLAYER.x, PLAYER.y);

            animation_phase = 0;
            is_animating = FALSE;
        }
    }
    animation_timer++;
}

void update_level()
{
    vram_addr = get_bkg_xy_addr(7, 17);
    sprintf(level_buffer, "%u", level_text);

    for (UINT8 *ptr = level_text; *ptr != 0; ptr++)
    { // when for loop hits a 0 (NOT A '0', WHICH IS ASCII), the loop ends
        set_vram_byte(vram_addr, *ptr + 96);
        vram_addr++;
    }
    vram_addr = get_bkg_xy_addr(13, 17);
    // current level in init_level();
    set_vram_byte(vram_addr, current_level + 96);
}
void update_player1_player2()
{
    vram_addr = get_bkg_xy_addr(0, 15);
    for (UINT8 *ptr = PLAYER1UP_text; *ptr != 0; ptr++)
    { // when for loop hits a 0 (NOT A '0', WHICH IS ASCII), the loop ends
        set_vram_byte(vram_addr, *ptr + 96);
        vram_addr++;
    }
}
void update_score()
{
    UINT8 vram_offset = 96;
    if (score < 100)
    {
        vram_addr = get_bkg_xy_addr(3, 16);
    }
    else if (score >= 100 && score < 1000)
    {
        vram_addr = get_bkg_xy_addr(2, 16);
    }
    else if (score >= 1000 && score < 10000)
    {
        vram_addr = get_bkg_xy_addr(1, 16);
    }
    else if (score > 10000)
        vram_addr = get_bkg_xy_addr(0, 16);
    // sprintf maxes at 16 bits on GAMEBOY
    sprintf(score_buffer, "%u", score); // stores the ascii code value into a buffer, one value per score_buffer[] item (SEE score_buffer[16] above)

    for (UINT8 *ptr = score_buffer; *ptr != 0; ptr++)
    { // when for loop hits a 0 (NOT A '0', WHICH IS ASCII), the loop ends
        set_vram_byte(vram_addr, *ptr + vram_offset);
        vram_addr++;
    }
}
void update_frog_lives()
{
    UINT8 frog_life_pos_x = 0;
    for (UINT8 i = lives; i != 0; i--)
    {
        set_bkg_tile_xy(frog_life_pos_x, 17, 47); // fill frog lives
        frog_life_pos_x++;
    }
    set_bkg_tile_xy(lives, 17, 0x00); // set blank tile for frog life loss
}
void reset_frog()
{
    update_frog_lives();
    is_moving = FALSE;
    move_x = move_y = 0;
    PLAYER.x = 56;  // 56
    PLAYER.y = 108; // 108
    y_min = PLAYER.y;
    PLAYER.position = ON_NOTHING;
    PLAYER.direction = UP;

    move_metasprite(
        frogger_metasprites[0], 0, 0, PLAYER.x, PLAYER.y);
}
void kill_frog()
{
    // animate frog death here//
    if (lives != 0)
    {
        lives -= 1;
    }
    else // GAME OVER
    {
        GAMESTATE = gameover;
        return;
    }
    reset_frog();
}
void reset_timer()
{
    timer = (UINT16)-32; // converts -32 to equivilent Unsigned value
    timer_tick = 0;
    set_bkg_tiles(5, 16, 10, 1, RESET_TIMER);
}

void init_level()
{
    set_sprite_data(0, 24, frogger_tiles);
    set_bkg_data(0, 47, BKG_TILES);
    set_bkg_data(47, 1, FROG_LIVES); // TESTING FROG LIFE UPDATE
    set_bkg_data(0x80, 68, FONT);
    set_bkg_tiles(0, 0, 32, 32, BKG_MAP);
    set_bkg_tile_xy(4, 16, 0x90); // set furthest '0' on the righthand side of score on Stage 1 init only (is updated as soon as player gains points)
    turtles_diving = FALSE;
    score = 0;
    update_score();
    current_level = '1';
    update_level();
    reset_timer();
    reset_frog();
    update_player1_player2();
    update_frog_lives();
}
void move_frog()
{
    // (MOVE_X AND MOVE_Y ARE UPDATED IN update_move_xy)
    if (move_x != 0) // MOVES THE FROG + or - X FOR 1 FRAME
    {
        PLAYER.x += (move_x < 0 ? -1 : 1);
        // if (PLAYER.direction == LEFT)
        move_metasprite(
            frogger_up_down_animation[animation_phase], 0, 0, PLAYER.x, PLAYER.y);
        // else if (PLAYER.direction == RIGHT)
        // move_metasprite_hflip(
        //     frogger_up_down_animation[animation_phase], 0, 0, PLAYER.x, PLAYER.y);
    }
    else if (move_y != 0) // MOVES THE FROG + or - Y FOR 1 FRAME
    {
        PLAYER.y += (move_y < 0 ? -1 : 1);
        if (PLAYER.direction == UP)
            move_metasprite(
                frogger_up_down_animation[animation_phase], 0, 0, PLAYER.x, PLAYER.y);
        else if (PLAYER.direction == DOWN)
            move_metasprite_hflip(
                frogger_up_down_animation[animation_phase], 0, 0, PLAYER.x, PLAYER.y + 48);
        // else if (PLAYER.direction == DOWN)
        //     move_metasprite_vflip(
        //         frogger_up_down_animation[animation_phase], 0, 0, PLAYER.x, PLAYER.y);
    }
}
void update_move_xy()
{
    if (move_x != 0)
    {
        move_frog();
        move_x += move_x < 0 ? 1 : -1; //---- ADD 1 OR -1 TO THE CURRENT MOVE_X
    }
    else if (move_y != 0)
    {
        move_frog();
        move_y += move_y < 0 ? 1 : -1; // ADD 1 OR -1 TO THE CURRENT MOVE_Y
    }
    else if (move_x == 0 || move_y == 0)
    {                      //
        is_moving = FALSE; // ALLOWS JOY PRESS
        if (PLAYER.y < y_min)
        {
            score += 10;
            update_score();
            y_min = PLAYER.y;
        }
    }
}
void parallaxScroll()
{ // CAMERA Y POSITION IN TILE ROWS
    switch (LYC_REG)
    {
    case 0x00: // first three stationary rows
        move_bkg(0, 0);
        LYC_REG = 0x17;
        break;
    case 0x17: // LOG 1
        move_bkg(SCROLL_LOG1, 0);
        LYC_REG = 0x1F;
        break;
    case 0x1F: // TURLES 1
        move_bkg(SCROLL_TURTLE1, 0);
        LYC_REG = 0x27;
        break;
    case 0x27: // LOG 2 (fast)
        move_bkg(SCROLL_LOG2, 0);
        LYC_REG = 0x2F;
        break;
    case 0x2F: // LOG 3
        move_bkg(SCROLL_LOG3, 0);
        LYC_REG = 0x37;
        break;
    case 0x37: // TURTLES 2
        move_bkg(SCROLL_TURTLE2, 0);
        LYC_REG = 0x40;
        break;
    case 0x40: // MIDDLE WALL
        move_bkg(0, 0);
        LYC_REG = 0x48;
        break;
    case 0x48: // CAR 1 BUS
        move_bkg(SCROLL_CAR1, 0);
        LYC_REG = 0x50;
        break;
    case 0x50: // CAR 2 PINK
        move_bkg(SCROLL_CAR2, 0);
        LYC_REG = 0x58;
        break;
    case 0x58: // CAR 3
        move_bkg(SCROLL_CAR3, 0);
        LYC_REG = 0x60;
        break;
    case 0x60: // CAR 4
        move_bkg(SCROLL_CAR4, 0);
        LYC_REG = 0x68;
        break;
    case 0x68: // CAR 5
        move_bkg(SCROLL_CAR5, 0);
        LYC_REG = 0x6F;
        break;
    case 0x6F: // BOTTOM CURB AND BELOW, STATIONARY
        move_bkg(0, 0);
        LYC_REG = 0x00;
        break;
    }
}
void scroll_counters()
{
    if (scx_counter % 6 == 0)
    {
        SCROLL_LOG1 -= 1;    // LOG 1
        SCROLL_TURTLE1 += 1; // TURTLES 1
        SCROLL_TURTLE2 += 1; // TURTLES 2

        if (PLAYER.position == ON_TURTLE)
        {
            PLAYER.x -= 1;
            move_metasprite(
                frogger_metasprites[0], 0, 0, PLAYER.x, PLAYER.y);
        }
        if (PLAYER.position == ON_LOG1)
        {
            PLAYER.x += 1;
            move_metasprite(
                frogger_metasprites[0], 0, 0, PLAYER.x, PLAYER.y);
        }
    }
    if (scx_counter % 5 == 0)
    {
        SCROLL_LOG3 -= 1; // LOG 3
        if (PLAYER.position == ON_LOG3)
        {
            PLAYER.x += 1;
            move_metasprite(
                frogger_metasprites[0], 0, 0, PLAYER.x, PLAYER.y);
        }
    }
    if (scx_counter % 4 == 0)
    {                     // += moves the 'camera' to the right, resulting in objects on screen moving left
        SCROLL_LOG2 -= 1; // LOG 2
        SCROLL_CAR3 += 1; // CAR3
        SCROLL_CAR4 -= 1; // CAR4
        SCROLL_CAR5 += 1; // CAR5
        if (PLAYER.position == ON_LOG2)
        {
            PLAYER.x += 1;
            move_metasprite(
                frogger_metasprites[0], 0, 0, PLAYER.x, PLAYER.y);
        }
    }

    if (scx_counter % 3 == 0)
    {
        SCROLL_CAR1 += 1; // CAR 1 BUS
    }
    SCROLL_CAR2 -= 1; // CAR 2 PINK

    scx_counter++;
}
void drain_timer()
{ // WHEN PLAYER REACHES TOP, DRAIN THE REMAINING TIMER
    UINT8 current_tile, tile_offset, x_offset;

    tile_offset = (timer_tick) % 8; // 1, 2, 3, 4, 5, 6, 7, 8. Back to 0
    x_offset = timer_tick / (8);    // MULTIPLYING BY 8 GETS YOU THE TILE VALUE
    current_tile = get_bkg_tile_xy(6 + x_offset, 16);

    if (current_tile == 0x25 + tile_offset)
    {
        set_bkg_tile_xy(6 + x_offset, 16, 0x26 + tile_offset);
    }
    timer_tick++;
    score += 10;
    update_score();
    timer++;
    if (timer_tick == 64)
    { // TIMER ENDS // 2048 + 1 because of function loop ordering
        reset_timer();
        reset_frog();
        GAMESTATE = game;
    }
}
void win_check(UINT8 frogx, UINT8 frogy)
{
    UINT16 left_x, right_x, indexY, tileindex_L, tileindex_R;
    UINT8 left_offset, right_offset;

    left_offset = WATER_OFFSET_L;
    right_offset = WATER_OFFSET_R;

    indexY = (frogy + 8) / 8;             // CALCULATE THE FROG'S Y ROW, USING HIS CENTER Y
    left_x = (frogx + left_offset) / 8;   // CALCULATE THE LEFT OF FROG X
    right_x = (frogx + right_offset) / 8; // CALCULATE THE RIGHT OF FROG X

    tileindex_L = 32 * indexY + left_x;
    tileindex_R = 32 * indexY + right_x;

    if (COLLISION_MAP[tileindex_L] == 0x01 && COLLISION_MAP[tileindex_R] == 0x01)
    {
        set_bkg_tiles(1, 1, 2, 2, WIN_FROG);
        GAMESTATE = drain;
        hide_metasprite(frogger_metasprites[0], 0); // HIDE FROGGER
    }
    else if (COLLISION_MAP[tileindex_L] == 0x02 && COLLISION_MAP[tileindex_R] == 0x02)
    {
        set_bkg_tiles(5, 1, 2, 2, WIN_FROG);
        GAMESTATE = drain;
        hide_metasprite(frogger_metasprites[0], 0); // HIDE FROGGER
    }
    else if (COLLISION_MAP[tileindex_L] == 0x03 && COLLISION_MAP[tileindex_R] == 0x03)
    {
        set_bkg_tiles(9, 1, 2, 2, WIN_FROG);
        GAMESTATE = drain;
        hide_metasprite(frogger_metasprites[0], 0); // HIDE FROGGER
    }
    else if (COLLISION_MAP[tileindex_L] == 0x04 && COLLISION_MAP[tileindex_R] == 0x04)
    {
        set_bkg_tiles(13, 1, 2, 2, WIN_FROG);
        GAMESTATE = drain;
        hide_metasprite(frogger_metasprites[0], 0); // HIDE FROGGER
    }
    else if (COLLISION_MAP[tileindex_L] == 0x05 && COLLISION_MAP[tileindex_R] == 0x05)
    {
        set_bkg_tiles(17, 1, 2, 2, WIN_FROG);
        GAMESTATE = drain;
        hide_metasprite(frogger_metasprites[0], 0); // HIDE FROGGER
    }
    else
    {
        kill_frog(); // WALL SPLAT, KILL FROG
    }
}
void collide_check(UINT8 frogx, UINT8 frogy)
{
    UINT16 left_x, right_x, indexY;
    UINT8 left_offset, right_offset;
    if (PLAYER.y >= STREET)
    {
        left_offset = STREET_OFFSET_L;
        right_offset = STREET_OFFSET_R;
    }
    else
    {
        left_offset = WATER_OFFSET_L;
        right_offset = WATER_OFFSET_R;
    }

    indexY = (frogy + 8) / 8;                             // CALCULATE THE FROG'S Y ROW, USING HIS CENTER Y
    UINT8 *scroll_ptr = scroll_remap[indexY];             // POINT TO THE CURRENT UINT8 SCROLL X VALUE
    UINT8 vblank_offset = scroll_ptr ? *scroll_ptr : 0;   // IF POINTING TO NULL, THEN OFFSET IS 0. OTHERWISE, OFFSET EQUALS THE CURRENT SCROLL X VALUE
    left_x = (frogx + left_offset + vblank_offset) / 8;   // CALCULATE THE LEFT OF FROG X
    right_x = (frogx + right_offset + vblank_offset) / 8; // CALCULATE THE RIGHT OF FROG X

    // 1024 total VRAM tiles 32x32
    UINT16 left_tile = get_bkg_tile_xy(left_x & 31, indexY);   // '& 31' WRAPPER PREVENTS VALUE FROM GOING HIGHER THAN 31. IT LOOPS BACK TO 0 (binary AND)
    UINT16 right_tile = get_bkg_tile_xy(right_x & 31, indexY); // '& 31' WRAPPER PREVENTS VALUE FROM GOING HIGHER THAN 31. IT LOOPS BACK TO 0 (binary AND)

    //? PLAYER.y % 8 ==4

    if (PLAYER.y >= STREET)
    { // ON OR BELOW THE SIDEWALK
        if ((left_tile >= CAR_TILES_START && left_tile <= CAR_TILES_END) || (right_tile >= CAR_TILES_START && right_tile <= CAR_TILES_END))
        { // CHECK FOR ALL CAR TILE COLLISIONS
            kill_frog();
        }
    }
    else if (PLAYER.y == TURTLE1 || PLAYER.y == TURTLE2)
    {
        if ((left_tile >= TURTLE_TILES_START && left_tile <= DIVE_TILES_END || left_tile >= TURTLE_TILES_START && right_tile <= DIVE_TILES_END))
        {                                // CHECK ALL TURTLE TILES
            PLAYER.position = ON_TURTLE; // MOVES FROG AT TURTLE SPEED IN scroll_counters();
        }
        else
        {
            if (!is_moving) // KILL FROG ONLY AFTER IT COMPLETES ITS MOVEMENT
                kill_frog();
        }
    }
    else if (PLAYER.y == LOG1)
    {
        if ((left_tile >= LOG_TILES_START && left_tile <= LOG_TILES_END || left_tile >= LOG_TILES_START && right_tile <= LOG_TILES_END))
        {                              // CHECK ALL LOG TILES
            PLAYER.position = ON_LOG1; // MOVES FROG AT LOG SPEED IN scroll_counters();
        }
        else
        {
            if (!is_moving) // KILL FROG ONLY AFTER IT COMPLETES ITS MOVEMENT
                kill_frog();
        }
    }
    else if (PLAYER.y == LOG2)
    {
        if ((left_tile >= LOG_TILES_START && left_tile <= LOG_TILES_END || left_tile >= LOG_TILES_START && right_tile <= LOG_TILES_END))
        {                              // CHECK ALL LOG TILES
            PLAYER.position = ON_LOG2; // MOVES FROG AT LOG SPEED IN scroll_counters();
        }
        else
        {
            if (!is_moving) // KILL FROG ONLY AFTER IT COMPLETES ITS MOVEMENT
                kill_frog();
        }
    }
    else if (PLAYER.y == LOG3)
    {
        if ((left_tile >= LOG_TILES_START && left_tile <= LOG_TILES_END || left_tile >= LOG_TILES_START && right_tile <= LOG_TILES_END))
        {                              // CHECK ALL LOG TILES
            PLAYER.position = ON_LOG3; // MOVES FROG AT LOG SPEED IN scroll_counters();
        }
        else
        {
            if (!is_moving) // KILL FROG ONLY AFTER IT COMPLETES ITS MOVEMENT
                kill_frog();
        }
    }
    else if (PLAYER.y == WIN)
    {
        win_check(PLAYER.x, PLAYER.y);
    }
}
void animate_turtles()
{
    turtle_counter++; // REGULAR TURTLES ANIMATION TIMER
    dive_counter++;   // DIVING TURTLES ANIMATION TIMER
    UINT8 row1, row2;
    UINT8 regular_frame = NULL; // REGULAR TURTLE ANIMATION LOOP FRAMES

    // -------------------------- REGULAR TURTLES -------------------------- //
    if (turtle_counter % 16 == 0)
    {                                                          // ANIMATE ALL TURTLES FRAME EVERY 16 GAME LOOPS
        regular_frame = turtle_tiles[turtle_tile_index++ % 4]; // TURTLE TILE FRAME OF ANIMATION (1 % 4 = 1) ++ modifies the turtle_tile_index variable each loop
        for (UINT8 i = 0; i < 32; i++)
        {
            row1 = get_bkg_tile_xy(i, 4); // TURTLES1 ROW
            row2 = get_bkg_tile_xy(i, 7); // TURTLES2 ROW
            if (row1 >= TURTLE_TILES_START && row1 <= TURTLE_TILES_END)
            {
                set_bkg_tile_xy(i, 4, regular_frame); // TURTLES1 ROW
            }
            if (row2 >= TURTLE_TILES_START && row2 <= TURTLE_TILES_END)
            {
                set_bkg_tile_xy(i, 7, regular_frame); // TURTLES2 ROW
            }
        }
    }

    // -------------------------- DIVING TURTLES -------------------------- //
    if ((dive_counter == 48) && (!turtles_diving))
    {                          // BEGIN DIVING TURTLES ANIMATIONS AFTER 48 GAME LOOPS
        turtles_diving = TRUE; // SEE BELOW
    }
    if (turtles_diving)
    { // BEGIN DIVING ANIMATIONS
        if (turtle_counter % 16 == 0)
        {
            UINT8 diving_frame = dive_tiles[dive_tile_index++ % 7]; // TURTLE TILE FRAME OF ANIMATION (1 % 4 = 1) ++ modifies the turtle_tile_index variable each loop

            if (diving_frame == NULL)
            { // IF THE DIVER ANIMATION IS COMPLETED
                for (UINT8 *ptr = turtle_divers1; *ptr != NULL; ptr++)
                {
                    set_bkg_tile_xy(*ptr, 4, regular_frame); // RETURN ALL ROW 1 DIVING TURTLES TO ALL REGULAR TURTLES' ANIMATION FRAME
                }
                for (UINT8 *ptr = turtle_divers2; *ptr != NULL; ptr++)
                {
                    set_bkg_tile_xy(*ptr, 7, regular_frame); // RETURN ALL ROW 2 DIVING TURTLES TO ALL REGULAR TURTLES' ANIMATION FRAME
                }

                turtles_diving = FALSE; // END DIVING ANIMATION
                dive_counter = 0;       // RESET DIVE COUNTER
            }
            else
            { // ANIMATE DIVE ANIMATIONS
                for (UINT8 *ptr = turtle_divers1; *ptr != NULL; ptr++)
                {
                    set_bkg_tile_xy(*ptr, 4, diving_frame); // ROW 1 TURTLES
                }
                for (UINT8 *ptr = turtle_divers2; *ptr != NULL; ptr++)
                {
                    set_bkg_tile_xy(*ptr, 7, diving_frame); // ROW 2 TURTLES
                }
            }
        }
    }
}
void stage_timer()
{
    UINT8 current_tile, tile_offset, x_offset;

    if (timer != (UINT16)-32 && timer % 32 == 0)
    {
        tile_offset = (timer / 32) % 8; // 1, 2, 3, 4, 5, 6, 7, 8. Back to 0
        x_offset = timer / (32 * 8);    // MULTIPLYING BY 8 GETS YOU THE TILE VALUE
        current_tile = get_bkg_tile_xy(6 + x_offset, 16);

        if (current_tile == 0x25 + tile_offset)
        {
            set_bkg_tile_xy(6 + x_offset, 16, 0x26 + tile_offset);
        }
        timer_tick++; // 64 ticks = DRAINED TIMER
    }
    timer++;
    if (timer_tick == 64)
    { // TIMER ENDS // 2048 + 1 because of function loop ordering
        reset_timer();
        kill_frog();
    }
}

void main()
{
    STAT_REG = 0x45; // enable LYC=LY interrupt so that we can set a specific line it will fire at //
    LYC_REG = 0x00;

    CRITICAL
    {
        add_LCD(parallaxScroll);
    }
    set_interrupts(VBL_IFLAG | LCD_IFLAG);

    DISABLE_VBL_TRANSFER;
    OBP1_REG = 0b10011100;
    SPRITES_8x16; // MUST be 8x16 or 8x8. Can change in different scenes only
    SHOW_BKG;
    SHOW_SPRITES;
    DISPLAY_ON;

    GAMESTATE = game;

    init_level();

    while (1)
    {
        last_joy = joy;
        joy = joypad();
        // -------------------- COLLISION CHECK -------------------------------//
        collide_check(PLAYER.x, PLAYER.y);

        // -------------------- BUTTON INPUT -------------------------------//
        if (GAMESTATE == game)
        { // REGULAR GAME TIME
            if (!is_moving)
            {
                switch (joy)
                {
                case J_LEFT:
                    if (CHANGED_BUTTONS & J_LEFT)
                    {
                        is_moving = TRUE;
                        is_animating = TRUE;
                        move_x = -12;
                        PLAYER.direction = LEFT;
                    }
                    break;
                case J_RIGHT:
                    if (CHANGED_BUTTONS & J_RIGHT)
                    {
                        is_moving = TRUE;
                        is_animating = TRUE;
                        move_x = 12;
                        PLAYER.direction = RIGHT;
                    }
                    break;
                case J_UP:
                    if (CHANGED_BUTTONS & J_UP)
                    {
                        is_moving = TRUE;
                        is_animating = TRUE;
                        move_y = -8;
                        PLAYER.position = ON_NOTHING;
                        PLAYER.direction = UP;
                    }
                    break;
                case J_DOWN:
                    if ((PLAYER.y <= 100) && (CHANGED_BUTTONS & J_DOWN))
                    { // PREVENT FROG FROM GOING BELOW SPAWN POINT
                        is_moving = TRUE;
                        is_animating = TRUE;
                        move_y = 8;
                        PLAYER.position = ON_NOTHING;
                        PLAYER.direction = DOWN;
                    }
                    break;
                }
            }
            // --------------------STAGE TIMER -------------------------------//
            stage_timer(); // REGULAR TIMER
        }
        else if (GAMESTATE == drain) // TIMER IS DRAINING, WIN CONDITION. DRAINS REMAINING TIMER INTO POINTS
            drain_timer();           // DRAIN TIMER // HIGH SPEED
        else if (GAMESTATE == gameover)
        {
            return;
        }

        // --------------------MOVE FROG -------------------------------//
        update_move_xy();
        // ---------------- SCROLL COUNTERS --------------------------- //
        scroll_counters();
        // ---------------- ANIMATE FROG --------------------------- //
        if (is_animating)
            render_animations();
        // ---------------- ANIMATE TURTLES --------------------------- //
        animate_turtles();
        // -------------------- DEBUG -------------------------------//
        if (joy & J_SELECT)
        {
            reset_frog();
        }
        // debug
        // printf("%u ", timer);
        // // debug
        // if (joy & J_A) {
        //     // printf("%u\n", PLAYER.y);
        //     set_bkg_tile_xy(4, 4, 0x11);
        // }

        wait_vbl_done();
        refresh_OAM();
    }
}