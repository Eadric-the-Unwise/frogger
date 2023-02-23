#include <gb/gb.h>
#include <gbdk/metasprites.h>
#include <stdio.h>

#include "../res/tiles/bkg_map.h"
#include "../res/tiles/bkg_tiles.h"
#include "../res/tiles/frogger.h"

#define CHANGED_BUTTONS (last_joy ^ joy)

typedef struct GameCharacter {
    UBYTE spawn;
    INT16 x;
    INT16 y;
    UINT8 width;
    UINT8 height;

} GameCharacter;