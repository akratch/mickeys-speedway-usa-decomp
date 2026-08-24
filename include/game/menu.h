#ifndef _GAME_MENU_H_
#define _GAME_MENU_H_

#include "PR/ultratypes.h"

s32 frontGetScreenMode(void);
void frontStoreScreenMode(void);
s32 frontGetLevelScreenMode(void);
s8 frontGetWideAdjust(void);
void frontSetWideAdjust(s32 offset);

#endif
