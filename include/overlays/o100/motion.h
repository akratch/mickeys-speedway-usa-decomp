#ifndef OVERLAYS_O100_MOTION_H
#define OVERLAYS_O100_MOTION_H

#include "PR/ultratypes.h"

typedef struct Overlay100Vec3 {
    f32 x;
    f32 y;
    f32 z;
} Overlay100Vec3;

typedef struct Overlay100Motion {
    s16 bank;
    u8 nextBank;
    u8 phase;
    s16 count;
    s16 duration;
    s16 remaining;
    u8 colorB0;
    u8 colorB1;
    u8 colorB2;
    u8 colorA0;
    u8 colorA1;
    u8 colorA2;
    Overlay100Vec3 *velocity;
    Overlay100Vec3 *frames[3];
} Overlay100Motion;

#endif
