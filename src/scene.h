#include <gb/gb.h>
#include <gbdk/metasprites.h>
#include <stdio.h>

#include "../res/tiles/bkg_map.h"
#include "../res/tiles/bkg_tiles.h"
#include "../res/tiles/frogger.h"
#include "../res/tiles/frogger_16.h"
#define CHANGED_BUTTONS (last_joy ^ joy)

typedef struct GameCharacter {
    UBYTE spawn;
    INT16 x;
    INT16 y;
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
