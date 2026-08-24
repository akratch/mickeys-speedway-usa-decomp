#ifndef _GAME_GAMEVI_H_
#define _GAME_GAMEVI_H_

#include "PR/ultratypes.h"

void viFrameRateReset(void);
s32 viGetVideoMode(void);
s8 viGetWideAdjust(void);
void viSetWideAdjust(s32 offset);
void viSetTrippleBuffer(s32 resolutionIndex);
s32 viChangeBuffers(void);
s32 viDisplayingScreen0(void);
void fb_memcpy(u8 *src, u8 *dest, s32 len);

#endif /* _GAME_GAMEVI_H_ */
