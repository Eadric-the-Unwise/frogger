#pragma bank 1

//AUTOGENERATED FILE FROM png2asset

#include <stdint.h>
#include <gbdk/platform.h>
#include <gbdk/metasprites.h>

BANKREF(tophat_frog)

const palette_color_t tophat_frog_palettes[5] = {
	RGB8(255, 255, 255), RGB8(172, 224, 139), RGB8(52, 104, 86), RGB8(8, 24, 32)
};

const uint8_t tophat_frog_tiles[64] = {
	0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x07,0x06,0x0f,0x1f,0x3f,0x0f,0x0a,0x7f,0x7c,0x78,0x6f,0x7b,0x7e,0x3c,0x7f,0x0f,0x1f,0x1b,0x3f,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xc0,0xc0,0x40,0xf0,0xf8,0xe0,0xa0,0xfe,0x7e,0x16,0xfa,0xd4,0x7e,0x38,0xfc,0xf0,0xf8,0x58,0xfc,0x00,0x00,0x00,0x00,0x00,0x00
};

const metasprite_t tophat_frog_metasprite0[] = {
	METASPR_ITEM(16, 8, 0, 0), METASPR_ITEM(0, 8, 2, 0), METASPR_TERM
};

const metasprite_t* const tophat_frog_metasprites[1] = {
	tophat_frog_metasprite0
};