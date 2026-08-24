#ifndef GAME_ANIM_H
#define GAME_ANIM_H

#include "PR/ultratypes.h"

extern s32 *D_800D6D54;
extern u8 *D_800D6D58;
extern u32 D_800D6D5C;
extern void *D_8007D698;
extern void *D_8007D69C;
extern void *D_8007D6A0;

void func_80050000(s32 *stream);
s32 func_80050024(u32 bitCount);
s32 func_800500A4(u32 bitCount);
void func_8005017C(void);

#endif
