#ifndef __SOUND__
#define __SOUND__

#define UNMUTE 0
#define MUTE 1

#include "hUGEdriver.h"

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
// void hUGE_init_nonbanked();

#endif