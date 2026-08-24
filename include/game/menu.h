#ifndef _GAME_MENU_H_
#define _GAME_MENU_H_

#include "PR/ultratypes.h"

s32 frontGetScreenMode(void);
void frontStoreScreenMode(void);
u8 frontRecallScreenMode(void);
s32 frontGetLevelScreenMode(void);
s8 frontGetWideAdjust(void);
void frontSetWideAdjust(s32 offset);
u16 frontGetSfxVolume(void);
void frontSetSfxVolume(s32 volume);

#endif
