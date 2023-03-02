#include <gb/gb.h>
#include <gbdk/metasprites.h>
#include <stdio.h>

#include "../res/tiles/bkg_map.h"
#include "../res/tiles/bkg_tiles.h"
#include "../res/tiles/collision_map.h"
#include "../res/tiles/frogger.h"
#include "../res/tiles/frogger_16.h"
#include "../res/tiles/win_frog.h"

#define CHANGED_BUTTONS (last_joy ^ joy)

#define WIN 12
#define LOG1 20
#define TURTLE1 28
#define LOG2 36
#define LOG3 44
#define TURTLE2 52
#define STREET 60

#define STREET_OFFSET_L 2
#define STREET_OFFSET_R 13
#define WATER_OFFSET_L 6
#define WATER_OFFSET_R 9

#define CAR_TILES_START 0x05
#define CAR_TILES_END 0x0F

#define TURTLE_TILES_START 0x10
#define TURTLE_TILES_END 0x12

#define LOG_TILES_START 0x13
#define LOG_TILES_END 0x15

typedef enum {
    ON_NOTHING,
    ON_TURTLE,
    ON_LOG3,
    ON_LOG2,
    ON_LOG1
} position_e;

typedef struct GameCharacter {
    UBYTE spawn;
    INT16 x;
    INT16 y;
    position_e position;
} GameCharacter;

UINT8 scroll[10];  // VALUES THE AMOUNT IT HAS SCROLLED

#define SCROLL_LOG1 scroll[0]
#define SCROLL_TURTLE1 scroll[1]
#define SCROLL_LOG2 scroll[2]
#define SCROLL_LOG3 scroll[3]
#define SCROLL_TURTLE2 scroll[4]
#define SCROLL_CAR1 scroll[5]
#define SCROLL_CAR2 scroll[6]
#define SCROLL_CAR3 scroll[7]
#define SCROLL_CAR4 scroll[8]
#define SCROLL_CAR5 scroll[9]

// EVERY Y ROW STARTIN FROM TOP TO BOTTOM
// POINTS TO THE CURRENT X VALUE OF THE ROW THAT HAS SCROLLED (THIS IS LATER USED AS THE COLLISION OFFSET)
UINT8 *scroll_remap[18] = {NULL, NULL, NULL, &SCROLL_LOG1, &SCROLL_TURTLE1, &SCROLL_LOG2, &SCROLL_LOG3, &SCROLL_TURTLE2, NULL, &SCROLL_CAR1, &SCROLL_CAR2, &SCROLL_CAR3, &SCROLL_CAR4, &SCROLL_CAR5, NULL, NULL, NULL, NULL};
