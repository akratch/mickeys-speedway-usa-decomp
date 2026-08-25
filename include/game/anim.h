#ifndef GAME_ANIM_H
#define GAME_ANIM_H

#include "PR/ultratypes.h"

extern s32 *D_800D6D54;
extern u8 *D_800D6D58;
extern u32 D_800D6D5C;
extern void *D_8007D698;
extern void *D_8007D69C;
extern s32 D_8007D6A0;
extern s8 D_8007D6C0[];
extern s32 *D_8007D68C;
extern s32 D_8007D6A4;
extern s32 D_8007D6A8;
extern f32 D_8007D6AC;
extern u32 osRomBase;
extern void *D_8007D680;
extern s32 D_8007D688;
typedef struct AnimPathObjectTarget {
    u8 pad0[0x132];
    s16 unk132;
} AnimPathObjectTarget;

typedef struct AnimPathObject {
    u8 pad0[6];
    s16 unk6;
    u8 pad8[0x50];
    AnimPathObjectTarget *unk58;
} AnimPathObject;

typedef struct AnimPathNode {
    u8 pad0[0x24];
    struct AnimPathNode *previous;
    struct AnimPathNode *next;
} AnimPathNode;

typedef struct AnimPath {
    u8 pad0[8];
    AnimPathObject *unk8;
    u8 padC[0xA];
    u8 flags;
    u8 nodeCount;
    u8 pad18[8];
    AnimPathNode *nodes;
} AnimPath;

extern AnimPath **D_800D6B00;
extern void *D_800D6B18[];
extern void *D_800D6B58[];

typedef struct AnimPauseSlot {
    s16 unk0;
    u8 pad2[9];
    u8 unkB;
} AnimPauseSlot;

extern AnimPauseSlot D_800D6D18[5];

typedef struct FmvPlayer {
    s8 unk0;
    u8 pad1[0x10];
    s32 unk14;
    s32 unk18;
    s32 unk1C;
    s32 unk20;
} FmvPlayer;

typedef struct AnimVec3f {
    f32 x;
    f32 y;
    f32 z;
} AnimVec3f;

typedef struct HitCopySource {
    u8 pad0[0x18];
    AnimVec3f current;
    AnimVec3f previous;
} HitCopySource;

typedef struct HitCopyTarget {
    u8 pad0[0x14];
    f32 unk14;
    f32 unk18;
    f32 unk1C;
    f32 unk20;
    f32 unk24;
} HitCopyTarget;

typedef struct HitCopyState {
    u8 pad0[0xC];
    AnimVec3f position;
    u8 pad18[0x30];
    HitCopySource *source;
    u8 pad4C[0x18];
    HitCopyTarget *target;
} HitCopyState;

extern void *D_800D76D0[2];
extern FmvPlayer D_800D76D8[2];

void func_80050000(s32 *stream);
s32 func_80050024(u32 bitCount);
s32 func_800500A4(u32 bitCount);
void func_8005013C(void);
void func_8005017C(void);
s32 func_800501AC(u16 *entry);
s32 func_800501C8();
void func_8005027C(void);
void func_800502CC(u8 pathIndex);
void func_80050AD4(u8 pathIndex);
void func_80006EA0(void *ptr);
void func_80050348(s32 pathIndex);
void func_8005055C();
void animseqStartPath(u8 pathIndex);
void animseqStopPath(u8 pathIndex);
void animseqInitGroup(void);
void animseqPlay(void);
AnimPath *func_800508B4(u8 pathIndex);
void animseqLockPath(u8 pathIndex);
void animseqUnLockPath(u8 pathIndex);
u32 func_8005077C(u8 pathIndex);
void animseqHoldPath(u8 pathIndex);
void amSndStop(void *ptr);
void func_80050D50(void);
void mmFree(void *ptr);
void animseqFreeLevelData(void);
void func_80050E9C(void);
void func_800534C0();
s32 TrapDanglingJump();
void func_800534EC(s32 arg0);
void animseqResetGroup(void);
void *piRomLoad(s32 resourceId);
void fmvInit(void);
void func_80055E50(HitCopyState *first, HitCopyState *second, f32 unused);
void func_800572AC(HitCopyState *state, void *unused, AnimVec3f *position,
                   f32 unusedFloat);
void func_80057350(HitCopyState *state, void *unused, AnimVec3f *position,
                   f32 unusedFloat);

#endif
