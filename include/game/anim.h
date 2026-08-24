#ifndef GAME_ANIM_H
#define GAME_ANIM_H

#include "PR/ultratypes.h"

extern s32 *D_800D6D54;
extern u8 *D_800D6D58;
extern u32 D_800D6D5C;
extern void *D_8007D698;
extern void *D_8007D69C;
extern void *D_8007D6A0;
extern s8 D_8007D6C0[];
extern s32 *D_8007D68C;
extern s32 D_8007D6A4;
typedef struct AnimPath {
    u8 pad0[0x16];
    u8 flags;
} AnimPath;

extern AnimPath **D_800D6B00;

typedef struct AnimPauseSlot {
    s16 unk0;
    u8 pad2[9];
    u8 unkB;
} AnimPauseSlot;

extern AnimPauseSlot D_800D6D18[5];

void func_80050000(s32 *stream);
s32 func_80050024(u32 bitCount);
s32 func_800500A4(u32 bitCount);
void func_8005013C(void);
void func_8005017C(void);
s8 func_800501AC(u16 *entry);
void *func_800501C8(void **cursor);
void func_8005027C(void);
void func_80050348(s32 pathIndex);
void animseqInitGroup(void);
void animseqPlay(void);
AnimPath *func_800508B4(u8 pathIndex);
void animseqLockPath(u8 pathIndex);
void animseqUnLockPath(u8 pathIndex);
u32 func_8005077C(u8 pathIndex);
void func_800534C0(s32 i);

#endif
