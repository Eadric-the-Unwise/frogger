#include <gb/gb.h>
#include <gbdk/metasprites.h>
#include <stdio.h>

#include "../res/tiles/bkg_map.h"
#include "../res/tiles/bkg_tiles.h"
#include "../res/tiles/frogger.h"
#include "../res/tiles/frogger_16.h"
#define CHANGED_BUTTONS (last_joy ^ joy)

UINT8 scroll[10];

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

typedef struct GameCharacter {
    UBYTE spawn;
    INT16 x;
    INT16 y;
    UINT8 width;
    UINT8 height;

} GameCharacter;