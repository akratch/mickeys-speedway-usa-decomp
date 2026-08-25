#ifndef _GAME_MENU_H_
#define _GAME_MENU_H_

#include "PR/ultratypes.h"

u8 frontGetMode(void);
s32 frontGetLanguage(void);
void frontSetLanguage(s32 language);
s32 frontGetScreenMode(void);
void frontStoreScreenMode(void);
u8 frontRecallScreenMode(void);
s32 frontGetLevelScreenMode(void);
s8 frontGetWideAdjust(void);
void frontSetWideAdjust(s32 offset);
u32 frontGetStereoMode(void);
u16 frontGetSfxVolume(void);
void frontSetSfxVolume(s32 volume);
u16 frontGetBgmVolume(void);
void frontSetBgmVolume(s32 volume);
s32 frontGet2PlayerSplit(void);
void func_8003A544(s32 value);
s32 func_8003A550(void);
void func_8003A55C(s32 value);
void func_8003A590(void);

#endif
