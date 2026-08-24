#ifndef OVERLAY31_BUILD_LOOKUP_TABLES_H
#define OVERLAY31_BUILD_LOOKUP_TABLES_H

#include "PR/ultratypes.h"

typedef struct Overlay31IndexRecord {
    u8 a0;
    u8 a1;
    u8 a2;
    u8 a3;
    s16 b4;
    s16 b6;
    s16 b8;
    s16 bA;
    s16 bC;
    s16 bE;
    u8 c10;
    u8 c11;
    u8 c12;
    u8 c13;
    s16 d14;
    s16 d16;
    s16 d18;
    s16 d1A;
    s16 d1C;
    s16 d1E;
} Overlay31IndexRecord;

typedef struct Overlay31FloatPair {
    f32 first;
    f32 second;
} Overlay31FloatPair;

extern Overlay31IndexRecord *gOverlay31IndexRows[7][2];
extern Overlay31FloatPair *gOverlay31FloatRows[7];

extern void *overlay31AllocateReloc(s32 size, s32 tag);
extern f32 func_8002A8BC(s32 angle);
extern f32 func_8002A8C0(s32 angle);

#endif
