#include <gb/gb.h>
#include <gbdk/metasprites.h>
#include <stdio.h>
#include <string.h>
#include <rand.h>

#include "scene.h"
//------------GOALS-------------//
// ADD MORE RNG TO FLY WHEN IDLING ABOVE SIDEWALK
// OPTIMIZE WIN_CHECK
// PRACTICE BINARY OPERATORS ON PAPER
// debugger
// WIN SCREEN
// GAME OVER
// CGB PALLETES
// SOUND
// STAGE 2
GameCharacter_t PLAYER;                                  // FROG
GameCharacter_t LOG_FROG;                                // TOPHAT FROG
GameCharacter_t FLY;                                     // 200 pt FLY
UINT8 joy, last_joy;                                     // CHECKS FOR CURRENT AND PREVIOUS JOY INPUTS IN MAIN WHILE()
UBYTE is_moving, is_animating, is_dying, turtles_diving; // IS MOVING = LOCKS JOY WHILE FROG IS ANIMATION TO NEXT TILE // TURTLES DIVING = TURTLES CURRENTLY ANIMATING DIVE ANIMATION // IS ANIMATION = ANIMATES FROG UNTIL ANIMATION REACHES ITS END
INT8 move_x, move_y;                                     // IF NOT 0, MOVE FROG 1 PIXEL PER LOOP IN move_frog();
UINT8 scx_counter;                                       // VBLANK INTERRUPT COUNTER
UINT8 turtle_counter, dive_counter;                      // TURTLE ANIMATION COUNTER
UINT16 timer, fly_timer, fly_respawn_timer;              // STAGE TIMER // FLY TIMER
UINT8 timer_tick;                                        // TIMER TICK (8 TICKS PER TIMER BAR TILE)
UINT8 pause_tick;
UINT8 flash;     // 'glowing' flash effect animation timer in update_palette();
UBYTE GAMESTATE; // GAME, DRAIN (TIMER), GAMEOVER
UBYTE TIMERSTATE;

UINT8 turtle_divers1[] = {0x0A, 0x0B, NULL};       // ROW 1 TURTLES
UINT8 turtle_divers2[] = {0x10, 0x11, 0x12, NULL}; // ROW 2 TURTLES

UINT8 turtle_tiles[] = {TURTLE_TILES_START, 0x11, TURTLE_TILES_END, TURTLE_TILES_END};                                   // REGULAR TURTLE ANIMATION LOOP
UINT8 dive_tiles[] = {DIVE_TILES_START, DIVE_TILES_END, WATER_TILE, WATER_TILE, DIVE_TILES_END, DIVE_TILES_START, NULL}; // DIVING TURTLE ANIMATION
UINT8 turtle_tile_index, dive_tile_index;

cave_t cave[5];
UINT8 cave_fly_x[] = {7, 39, 71, 103, 135}; // 5 spawn locations of FLY spawn_fly();

UINT8 y_min;

uint8_t *vram_addr;

UINT8 lives = 4;
// UINT32 score = 87654321; //USE THIS IF HIGHER THAN 65535
UINT16 score;
UINT8 score_buffer[5]; // MAX SCORE 9999 = {'9', '9', '9', '9', 0}; // if score = 150, score_buffer[0] = '1',score_buffer[1] = '5', score_buffer[2] = '0', score_buffer[3] = 0

UINT8 PLAYER1UP_text[] = "1 UP";
UINT8 PLAYER2UP_text[] = "2 UP";

UINT8 level_text[] = "LEVEL ";
UINT8 pause_text[] = "PAUSE";
// UINT8 pause_clear_text[] = "     ";
UINT8 current_level;
UINT16 level_buffer[12];

// DISPLAY LEVEL 1, LEVEL 2, LEVEL 3 AT BOTTOM
UINT8 death_animation_timer = 1;
UINT8 animation_phase;
UINT8 death_animation_phase;

UINT16 seed;
UINT8 debug_rng;
UINT8 debug_fly_spawn;

BYTE overlap(INT16 r1_y, INT16 r1_x, INT16 l1_y, INT16 l1_x, INT16 r2_y, INT16 r2_x, INT16 l2_y, INT16 l2_x)
{ // BYTE IS SAME AS BOOLEAN (ONLY SHORTER NAME)
    // Standard rectangle-to-rectangle collision check

    if (l1_x == r1_x || l1_y == r1_y || l2_x == r2_x || l2_y == r2_y)
    {
        // the line cannot have positive overlap
        return 0x00U;
    }
    if ((l1_x >= r2_x) || (l2_x >= r1_x))
    {
        return 0x00U;
    }
    if ((r1_y >= l2_y) || (r2_y >= l1_y))
    {
        return 0X00U;
    }

    return 0x01U;
}
void remove_fly()
{
    FLY.spawn = FALSE;
    hide_metasprite(fly_metasprites[0], 4);
    fly_timer = 0;
    fly_respawn_timer = 0;
}
void kill_frog()
{
    move_x = 0;
    is_moving = FALSE;
    is_dying = TRUE;
    if (PLAYER.flash)
        PLAYER.flash = FALSE;
}
void render_animations()
{
    // if (animation_timer % 1 == 0)
    {
        // move_metasprite(frogger_up_down_animation[animation_phase], 0, 0, PLAYER.x, PLAYER.y);
        animation_phase++;
        if (frogger_up_down_animation[animation_phase] == NULL)
        {
            if (PLAYER.direction == DOWN)
                move_metasprite_hflip(
                    frogger_metasprites[0], 0, 0, PLAYER.x, PLAYER.y + HFLIP_OFFSET);
            else if (PLAYER.direction == UP)
                move_metasprite(frogger_metasprites[0], 0, 0, PLAYER.x, PLAYER.y);

            animation_phase = 0;
            is_animating = FALSE;
        }
    }
    // animation_timer++;
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
void update_frog_direction()
{ // UPDATES THE FROG'S POSITION WHEN ON TURTLES AND LOGS
    switch (PLAYER.direction)
    {
    case UP:
        move_metasprite(
            frogger_metasprites[0], 0, 0, PLAYER.x, PLAYER.y); // UP IDLE POSITION
        break;
    case DOWN:
        move_metasprite_hflip(
            frogger_metasprites[0], 0, 0, PLAYER.x, PLAYER.y + HFLIP_OFFSET); // DOWN IDLE POSITION
        break;
    case RIGHT:
        move_metasprite(
            frogger_metasprites[3], 0, 0, PLAYER.x, PLAYER.y); // RIGHT IDLE POSITION
        break;
    case LEFT:
        move_metasprite_vflip(
            frogger_metasprites[3], 0, 0, PLAYER.x + VFLIP_OFFSET, PLAYER.y); // LEFT IDLE POSITION
        break;
    }
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
void reset_timer()
{
    timer = (UINT16)-32; // converts -32 to equivilent Unsigned value
    timer_tick = 0;
    set_bkg_tiles(5, 16, 10, 1, RESET_TIMER);
}
void frog_death()
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
    reset_timer();
}
void init_level()
{
    set_sprite_data(0, frogger_TILE_COUNT, frogger_tiles);
    set_sprite_data(0x24, 4, log_frog_tiles);
    set_sprite_data(0x28, 4, fly_tiles);
    set_bkg_data(0, 47, BKG_TILES);
    set_bkg_data(47, 1, FROG_LIVES); // TESTING FROG LIFE UPDATE
    set_bkg_data(0x80, 68, FONT);
    set_bkg_tiles(0, 0, 32, 32, BKG_MAP);
    set_bkg_tile_xy(4, 16, 0x90); // set furthest '0' on the righthand side of score on Stage 1 init only (is updated as soon as player gains points)
    LOG_FROG.spawn = TRUE;        // Jumping TOPHAT FROG will jump on LOG3 until !spawn
    LOG_FROG.flash = TRUE;
    LOG_FROG.x = 196;
    LOG_FROG.y = 42;
    turtles_diving = FALSE;
    score = 0;
    update_score();
    current_level = '1';
    update_level();
    reset_timer();
    reset_frog();
    update_player1_player2();
    update_frog_lives();
    for (UINT8 i = 0; i != 5; i++) // CLEAR ALL CAVE empty FLAGS
    {
        cave[i].empty = TRUE;
    }
    FLY.y = 7;
    fly_respawn_timer = MAX_FLY_RESET_TIMER; // FLY WILL IMMEDIATELY SPAWN WHEN PLAYER.Y <=60. (HELPS PREVENT RNG MANIPULATION ON SLOW FLY SPAWN)
}
void move_frog()
{
    // (MOVE_X AND MOVE_Y ARE UPDATED IN update_move_xy)
    if (move_x != 0) // MOVES THE FROG + or - X FOR 1 FRAME
    {
        PLAYER.x += (move_x < 0 ? -1 : 1);

        if (PLAYER.direction == RIGHT)
        {
            move_metasprite(
                frogger_left_right_animation[animation_phase], 0, 0, PLAYER.x, PLAYER.y);
        }
        else if (PLAYER.direction == LEFT)
        {
            move_metasprite_vflip(
                frogger_left_right_animation[animation_phase], 0, 0, PLAYER.x + VFLIP_OFFSET, PLAYER.y); // OFFSET DUE TO PNG2ASSET.BAT OFFSET ERROR WHEN FLIPPED
        }
    }
    else if (move_y != 0) // MOVES THE FROG + or - Y FOR 1 FRAME
    {
        PLAYER.y += (move_y < 0 ? -1 : 1);
        if (PLAYER.direction == UP)
            move_metasprite(
                frogger_up_down_animation[animation_phase], 0, 0, PLAYER.x, PLAYER.y);
        else if (PLAYER.direction == DOWN)
            move_metasprite_hflip(
                frogger_up_down_animation[animation_phase], 0, 0, PLAYER.x, PLAYER.y + HFLIP_OFFSET); // OFFSET DUE TO PNG2ASSET.BAT OFFSET ERROR WHEN FLIPPED
    }
}
void animate_tophat()
{
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
    {                         //
        is_moving = FALSE;    // ALLOWS JOY PRESS
        if (PLAYER.y < y_min) // CHECK IF FROG HAS MOVED UP THE STAGE FOR 10 PTS
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
    case 0x17: // LOG 1 // TOP
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
    case 0x2F: // LOG 3 // BOTTOM
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
void update_palette()
{ // CORRECTLY UPDATES THE SPRITE COLOR PALETTES IF FLASHING IS ACTIVE. (move_metasprite OVERWRITES PROPERTY FLAGS, SO THIS CORRECTS THOSE UPDATES)

    UINT8 prop = 0;
    prop = get_sprite_prop(0);

    if (flash < 4)
    {
        prop &= ~(1 << 4); // ~0b00001000 // CLEARS THE 4TH BIT NO MATTER WHAT. 4TH BIT IS THE COLOR PALETTE (CHECK GBDK MANUAL BIT 4, BIT 5 etc.)
        //
    }
    else if (flash >= 4)
    {
        prop |= (1 << 4); // 0b00010000 // FLIPS THE 4TH BIT VALUE FROM 0 TO 1 ALTERNATIVELY. 4TH BIT IS THE COLOR PALETTE (CHECK GBDK MANUAL BIT 4, BIT 5 etc.)
    }

    if (flash >= 7)
        flash = 0;

    if (PLAYER.flash)
    {
        set_sprite_prop(0, prop);
        set_sprite_prop(1, prop);
    }
    else if (LOG_FROG.flash)
    {
        prop &= ~(1 << 5); // hflip - CLEARS BIT 5
        prop &= ~(1 << 6); // vflip - CLEARS BIT 6
        set_sprite_prop(2, prop);
        set_sprite_prop(3, prop);
    }
    flash++;
    // tophat_prop ^= 0b00010000; // FLIPS THE 4TH BIT VALUE FROM 0 TO 1 ALTERNATIVELY. 4TH BIT IS THE COLOR PALETTE (CHECK GBDK MANUAL BIT 4, BIT 5 etc.)
}
void scroll_counters()
{
    if (scx_counter % 6 == 0)
    {
        SCROLL_LOG1 -= 1;    // LOG 1 (TOP LOG)
        SCROLL_TURTLE1 += 1; // TURTLES 1
        SCROLL_TURTLE2 += 1; // TURTLES 2
        if (!is_dying)       // UPDATE FROG POSITION TO FOLLOW TURTLE SPEED, UNLESS DYING ANIMATION IS OCCURRING
        {
            if (PLAYER.position == ON_TURTLE)
            {
                PLAYER.x -= 1;
                update_frog_direction(); // UPDATES THE FROG'S POSITION WHEN ON TURTLES AND LOGS
            }
            if (PLAYER.position == ON_LOG1)
            {
                PLAYER.x += 1;
                update_frog_direction();
            }
        }
    }
    if (scx_counter % 5 == 0)
    {
        SCROLL_LOG3 -= 1; // LOG 3 (BOTTOM LOG)
        if (LOG_FROG.spawn)
        {
            LOG_FROG.x += 1; //
            move_metasprite(log_frog_metasprites[0], 0x24, 2, LOG_FROG.x & 255, LOG_FROG.y);
            update_palette();
        }
        if (!is_dying) // UPDATE FROG POSITION TO FOLLOW TURTLE SPEED, UNLESS DYING ANIMATION IS OCCURRING
        {
            if (PLAYER.position == ON_LOG3)
            {
                PLAYER.x += 1;
                update_frog_direction();
            }
        }
    }
    if (scx_counter % 4 == 0)
    {                     // += moves the 'camera' to the right, resulting in objects on screen moving left
        SCROLL_LOG2 -= 1; // LOG 2 (CENTER LOG)
        SCROLL_CAR3 += 1; // CAR3
        SCROLL_CAR4 -= 1; // CAR4
        SCROLL_CAR5 += 1; // CAR5
        if (!is_dying)    // UPDATE FROG POSITION TO FOLLOW TURTLE SPEED, UNLESS DYING ANIMATION IS OCCURRING
        {
            if (PLAYER.position == ON_LOG2)
            {
                PLAYER.x += 1;
                update_frog_direction();
            }
        }
    }

    if (scx_counter % 3 == 0)
    {
        SCROLL_CAR1 += 1; // CAR 1 BUS
    }
    SCROLL_CAR2 -= 1; // CAR 2 PINK

    scx_counter++;
}
void pause_game()
{
    GAMESTATE = pause;
    vram_addr = get_bkg_xy_addr(7, 15);
    for (UINT8 *ptr = pause_text; *ptr != 0; ptr++)
    { // when for loop hits a 0 (NOT A '0', WHICH IS ASCII), the loop ends
        set_vram_byte(vram_addr, *ptr + 96);
        vram_addr++;
    }
}
void unpause_game()
{
    GAMESTATE = game;
    vram_addr = get_bkg_xy_addr(7, 15);
    for (UINT8 i = 5; i != 0; i--)
    { // when for loop hits a 0 (NOT A '0', WHICH IS ASCII), the loop ends
        set_vram_byte(vram_addr, 0x00);
        vram_addr++;
    }
}
void drain_timer()
{ // WHEN PLAYER REACHES TOP, DRAIN THE REMAINING TIMER
    UINT8 current_tile, end_tile, tile_offset, x_offset;

    tile_offset = (timer_tick) % 8; // 1, 2, 3, 4, 5, 6, 7, 0. These are the 8 tiles of animation for timer.
    x_offset = timer_tick / (8);    // MULTIPLYING BY 8 GETS YOU THE TILE VALUE
    current_tile = get_bkg_tile_xy(6 + x_offset, 16);
    end_tile = get_bkg_tile_xy(13, 16); // FINAL TILE IN TIMER

    hide_metasprite(frogger_metasprites[0], 0); // HIDE FROGGER UNTIL TIMER DRAINS
    // if (FLY.spawn)
    //     remove_fly();

    if (current_tile == TIMER_TILE_FULL + tile_offset) // FIRST TIMER TILE + tile_offset
    {
        set_bkg_tile_xy(6 + x_offset, 16, 0x26 + tile_offset);
    }
    timer_tick++;
    score += 10;
    update_score();
    timer++;
    if (end_tile == TIMER_TILE_EMPTY)
    { // TIMER ENDS // 2048 + 1 because of function loop ordering
        timer_tick = 0;
        TIMERSTATE = reload;
    }
}
void reload_timer()
{
    UINT8 current_tile, start_tile, tile_offset, x_offset;

    tile_offset = (timer_tick) % 8;
    x_offset = timer_tick / (8); // MULTIPLYING BY 8 GETS YOU THE TILE VALUE
    current_tile = get_bkg_tile_xy(13 - x_offset, 16);
    start_tile = get_bkg_tile_xy(6, 16); // FINAL TILE IN TIMER

    if (current_tile == TIMER_TILE_EMPTY - tile_offset) // FIRST TIMER TILE + tile_offset
    {
        set_bkg_tile_xy(13 - x_offset, 16, 0x2C - tile_offset);
    }
    timer_tick++;
    if (start_tile == TIMER_TILE_FULL)
    {
        reset_timer();
        reset_frog();
        TIMERSTATE = tick;
    }
}
void pts_200_flash()
{
    score += 200;
    update_score();
    PLAYER.flash = FALSE;
}
void pts_200_fly()
{
    score += 200;
    update_score();
    remove_fly();
}
void collide_npc(GameCharacter_t *npc)
{
    UINT8 PLAYER_L, PLAYER_R, PLAYER_T, PLAYER_B, NPC_L, NPC_R, NPC_T, NPC_B;

    PLAYER_L = PLAYER.x + HITBOX_OFFSET_L; // PLAYER LEFT X
    PLAYER_R = PLAYER.x + HITBOX_OFFSET_R; // PLAYER RIGHT X
    PLAYER_T = PLAYER.y;                   // PLAYER TOP Y
    PLAYER_B = PLAYER.y + 10;              // PLAYER BOTTOM Y
    NPC_L = (npc->x + 2 & 255);            // NPC LEFT X
    NPC_R = ((npc->x + 14) & 255);         // NPC RIGHT X
    NPC_T = npc->y;                        // NPC TOP Y
    NPC_B = npc->y + 10;                   // NPC BOTTOM Y

    // if (PLAYER_L <= TOPHAT_R && PLAYER_R >= TOPHAT_R || PLAYER_R >= TOPHAT_L && PLAYER_L <= TOPHAT_L) // X CROSSOVER COLLISION
    if (overlap(PLAYER.y, PLAYER_R, PLAYER.y + 10, PLAYER_L, NPC_T, NPC_R, NPC_B, NPC_L) == 0x01U)
    {
        if (PLAYER.y == LOG3) // LOG FROG
        {
            npc->spawn = FALSE;
            npc->flash = FALSE;
            hide_metasprite(log_frog_metasprites[0], 2);
            PLAYER.flash = TRUE;
        }
        else if (PLAYER.y == WIN) // CAVE
        {
            if (FLY.spawn)
            {
                pts_200_fly(); // APPLY 200 PTS AND REMOVE_FLY
            }
        }
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
        if (CAVE1.empty)
        {
            CAVE1.empty = FALSE;
            if (PLAYER.flash)
                pts_200_flash();
            // if (FLY.spawn)
            //     collide_npc(&FLY);
            set_bkg_tiles(1, 1, 2, 2, WIN_FROG);
            TIMERSTATE = drain;
            hide_metasprite(frogger_metasprites[0], 0); // HIDE FROGGER
        }
        else
        {
            kill_frog(); // WALL SPLAT, KILL FROG
        }
    }
    else if (COLLISION_MAP[tileindex_L] == 0x02 && COLLISION_MAP[tileindex_R] == 0x02)
    {
        if (CAVE2.empty)
        {
            CAVE2.empty = FALSE;
            if (PLAYER.flash)
                pts_200_flash();
            // if (FLY.spawn)
            //     collide_npc(&FLY);
            set_bkg_tiles(5, 1, 2, 2, WIN_FROG);
            TIMERSTATE = drain;
            hide_metasprite(frogger_metasprites[0], 0); // HIDE FROGGER
        }
        else
        {
            kill_frog(); // WALL SPLAT, KILL FROG
        }
    }
    else if (COLLISION_MAP[tileindex_L] == 0x03 && COLLISION_MAP[tileindex_R] == 0x03)
    {
        if (CAVE3.empty)
        {
            CAVE3.empty = FALSE;
            if (PLAYER.flash)
                pts_200_flash();
            // if (FLY.spawn)
            //     collide_npc(&FLY);
            set_bkg_tiles(9, 1, 2, 2, WIN_FROG);
            TIMERSTATE = drain;
            hide_metasprite(frogger_metasprites[0], 0); // HIDE FROGGER
        }
        else
        {
            kill_frog(); // WALL SPLAT, KILL FROG
        }
    }
    else if (COLLISION_MAP[tileindex_L] == 0x04 && COLLISION_MAP[tileindex_R] == 0x04)
    {
        if (CAVE4.empty)
        {
            CAVE4.empty = FALSE;
            if (PLAYER.flash)
                pts_200_flash();
            // if (FLY.spawn)
            //     collide_npc(&FLY);
            set_bkg_tiles(13, 1, 2, 2, WIN_FROG);
            TIMERSTATE = drain;
            hide_metasprite(frogger_metasprites[0], 0); // HIDE FROGGER
        }
        else
        {
            kill_frog(); // WALL SPLAT, KILL FROG
        }
    }
    else if (COLLISION_MAP[tileindex_L] == 0x05 && COLLISION_MAP[tileindex_R] == 0x05)
    {
        if (CAVE5.empty)
        {
            CAVE5.empty = FALSE;
            if (PLAYER.flash)
                pts_200_flash();
            // if (FLY.spawn)
            //     collide_npc(&FLY);
            set_bkg_tiles(17, 1, 2, 2, WIN_FROG);
            TIMERSTATE = drain;
            hide_metasprite(frogger_metasprites[0], 0); // HIDE FROGGER
        }
        else
        {
            kill_frog(); // WALL SPLAT, KILL FROG
        }
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
            ;
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
            ;
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
            ;
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
            ;
        }
    }
    else if (PLAYER.y == LOG3)
    {
        if (LOG_FROG.spawn) // 400 pt FROG
            collide_npc(&LOG_FROG);

        if ((left_tile >= LOG_TILES_START && left_tile <= LOG_TILES_END || left_tile >= LOG_TILES_START && right_tile <= LOG_TILES_END))
        {                              // CHECK ALL LOG TILES
            PLAYER.position = ON_LOG3; // MOVES FROG AT LOG SPEED IN scroll_counters();
        }
        else
        {
            if (!is_moving) // KILL FROG ONLY AFTER IT COMPLETES ITS MOVEMENT
                kill_frog();
            ;
        }
    }
    else if (PLAYER.y == WIN)
    {
        win_check(PLAYER.x, PLAYER.y);
        if (FLY.spawn)
            collide_npc(&FLY);
    }
}
void animate_turtles()
{
    turtle_counter++;           // REGULAR TURTLES ANIMATION TIMER
    dive_counter++;             // DIVING TURTLES ANIMATION TIMER
    UINT8 row1, row2;           //
    UINT8 regular_frame = NULL; // REGULAR TURTLE ANIMATION LOOP FRAMES

    // -------------------------- REGULAR TURTLES -------------------------- //
    if (turtle_counter % 16 == 0)
    {                                                          // ANIMATE ALL TURTLES FRAME EVERY 16 GAME LOOPS
        regular_frame = turtle_tiles[turtle_tile_index++ % 4]; // TURTLE TILE FRAME OF ANIMATION (1 % 4 = 1) ++ modifies the turtle_tile_index variable each loop
        for (UINT8 i = 0; i < 32; i++)
        {
            row1 = get_bkg_tile_xy(i, 4);                               // TURTLES1 ROW
            row2 = get_bkg_tile_xy(i, 7);                               // TURTLES2 ROW
            if (row1 >= TURTLE_TILES_START && row1 <= TURTLE_TILES_END) // IF TILES ARE AMONG THE 'REGULAR' TURTLES, UPDATE THEIR ANIMATION VIA regular frame (turtle_tile_index)
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
        // reset_timer();
        kill_frog();
    }
}
void render_death_animation()
{
    if (death_animation_timer % 4 == 0)
    {
        move_metasprite(frogger_death_animation[death_animation_phase], 0, 0, PLAYER.x, PLAYER.y);
        death_animation_phase++;

        if (frogger_death_animation[death_animation_phase] == NULL)
        {
            // move_metasprite(frogger_metasprites[0], 0, 0, PLAYER.x, PLAYER.y);
            frog_death();
            death_animation_phase = 0;
            is_animating = FALSE;
            is_dying = FALSE;

            // break;
        }
    }
    death_animation_timer++;
    // move_metasprite(frogger_metasprites[6], 0, 0, PLAYER.x, PLAYER.y);
}
void edge_death(UINT8 death_pos_x)
{
    PLAYER.x = death_pos_x;
    kill_frog();
}
void spawn_fly()
{
    UINT8 timer_rng = timer % 5; // 1, 2, 3, 4, 0
    seed = DIV_REG;
    initrand(seed);
    UINT8 rng = (rand() + timer_rng) % 5;
    // debug_rng = rng;
    if (!cave[rng].empty) // CAVE IS ALREADY FILLED
    {
        for (UINT8 c = rng; cave[c].empty; c++) // search for next available cave, searching sequenctially from right of !empty cave, loop back to cave[0]
        {
            if (c > 4)
            {
                c = 0;
                // debug_rng = c;
            }
            if (cave[c].empty)
            {
                FLY.spawn = TRUE;
                move_metasprite(fly_metasprites[0], 0x28, 4, cave_fly_x[c], 7);
                FLY.x = cave_fly_x[c];
            }
        }
    }
    else
    { // EMPTY CAVE

        FLY.spawn = TRUE;
        move_metasprite(fly_metasprites[0], 0x28, 4, cave_fly_x[rng], 7);
        FLY.x = cave_fly_x[rng];
    }
}
void render_pause()
{
    last_joy = joy;
    joy = joypad();

    if (CLICKED(J_START))
    {
        unpause_game();
    }
    wait_vbl_done();
    refresh_OAM();
}
void render_game()
{
    last_joy = joy;
    joy = joypad();

    // -------------------- EDGE DEATH CHECK -------------------------------//
    if (PLAYER.x > 152)
        edge_death(144);
    else if (PLAYER.x < -8)
        edge_death(0);
    // -------------------- SPAWN FLY CHECK -------------------------------//
    if (fly_respawn_timer < MAX_FLY_RESET_TIMER) // FLY MUST WAIT 4 SECONDS BEFORE RESPAWNING //
        fly_respawn_timer++;
    if (PLAYER.y <= 60 && fly_respawn_timer >= MAX_FLY_RESET_TIMER && FLY.spawn == FALSE) //
        spawn_fly();

    if (FLY.spawn)
    {
        fly_timer++;
        if (fly_timer >= 600) // FLY LASTS ON SCREEN FOR 10 SECONDS // 600
            remove_fly();
    }
    // debug_fly_spawn = FLY.spawn;

    if (TIMERSTATE == tick) // REGULAR GAME TIME
    {                       // REGULAR GAME TIME
        // -------------------- COLLISION CHECK -------------------------------//
        collide_check(PLAYER.x, PLAYER.y);
        // -------------------- BUTTON INPUT -------------------------------//
        if (!is_moving && !is_dying)
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
                    PLAYER.position = ON_NOTHING; // UNTIL FROG LANDS
                    PLAYER.direction = UP;
                }
                break;
            case J_DOWN:
                if ((PLAYER.y <= 100) && (CHANGED_BUTTONS & J_DOWN))
                { // PREVENT FROG FROM GOING BELOW SPAWN POINT
                    is_moving = TRUE;
                    is_animating = TRUE;
                    move_y = 8;
                    PLAYER.position = ON_NOTHING; // UNTIL FROG LANDS
                    PLAYER.direction = DOWN;
                }
                break;
            }
        }
        // --------------------STAGE TIMER -------------------------------//
        if (!is_dying)
            stage_timer(); // REGULAR TIMER
    }
    else if (TIMERSTATE == drain) // DRAIN TIMER // HIGH SPEED
        drain_timer();
    else if (TIMERSTATE == reload) // RELOAD TIMER AFTER DRAIN_TIMER COMPLETES
        reload_timer();

    // --------------------MOVE FROG -------------------------------//
    update_move_xy();
    // ---------------- SCROLL COUNTERS --------------------------- //
    scroll_counters();
    // ---------------- ANIMATE FROG --------------------------- //
    if (is_animating && !is_dying)
        render_animations();
    if (is_dying)
        render_death_animation();
    // ---------------- ANIMATE TURTLES --------------------------- //
    animate_turtles();
    // ---------------- UPDATE PALETTE --------------------------- //
    if (PLAYER.flash || LOG_FROG.flash)
        update_palette();
    // -------------------- DEBUG -------------------------------//
    // if (joy & J_SELECT)
    // {
    //     reset_frog();
    // }
    // debug
    if (joy & J_A)
        LOG_FROG.spawn = TRUE;
    if (CLICKED(J_START))
    {
        pause_game();
    }
    // debug

    // {
    //     // printf("%u\n", PLAYER.y);
    //     set_bkg_tile_xy(4, 4, 0x11);
    // }

    wait_vbl_done();
    refresh_OAM();
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
    OBP0_REG = 0b11100100;
    OBP1_REG = 0b10011100;
    SPRITES_8x16; // MUST be 8x16 or 8x8. Can change in different scenes only
    SHOW_BKG;
    SHOW_SPRITES;
    DISPLAY_ON;

    GAMESTATE = game;
    TIMERSTATE = tick;

    init_level();

    while (1)
    {
        switch (GAMESTATE)
        {
        case game:
            render_game();
            break;
        case pause:
            render_pause();
            break;
            // case gameover:
            //     break;
        }
    }
}