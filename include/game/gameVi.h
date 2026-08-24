#ifndef _GAME_GAMEVI_H_
#define _GAME_GAMEVI_H_

#include "PR/ultratypes.h"

void viInit(void *scheduler);
void viAllocateZBuffer(s32 width, s32 height);
void viFreeZBuffer(s32 width, s32 height);
void viGetCurrentSize(s32 *width, s32 *height);
void viConvertXY(s32 *x, s32 *y);
void viFrameRateReset(void);
s32 viFrameSync(s32 mesg);
void viSetTiming(void);
s32 viGetVideoMode(void);
s8 viGetWideAdjust(void);
void viSetWideAdjust(s32 offset);
void viSetTrippleBuffer(s32 resolutionIndex);
s32 viChangeBuffers(void);
s32 viDisplayingScreen0(void);
void fb_swap(void);
void fb_memcpy(u8 *src, u8 *dest, s32 len);

#endif /* _GAME_GAMEVI_H_ */
