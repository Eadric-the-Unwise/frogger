#ifndef __SOUND_H_INCLUDE__
#define __SOUND_H_INCLUDE__

#include "hUGEdriver.h"

#define UNMUTE 0
#define MUTE 1

#define SFX_CH_RETRIGGER 0b11000000 // ???

extern const unsigned char SFX_0[];
extern const unsigned char SFX_1[];
extern const unsigned char SFX_2[];
extern const unsigned char SFX_3[];
extern const unsigned char SFX_4[];
extern const unsigned char SFX_5[];
extern const unsigned char SFX_6[];
extern const unsigned char SFX_7[];

extern unsigned char Sound_ChannelResumeDelay[4];

void sfx_play(unsigned char *sfx_id);
void sfx_update(void);
void hUGE_init_nonbanked(UINT8 music_bank, const hUGESong_t *song);

void mute_channels();
void music_play();
void music_resume();
void music_pause();

#endif