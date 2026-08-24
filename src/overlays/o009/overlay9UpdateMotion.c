#include "PR/ultratypes.h"

typedef struct O9Result {
    s16 angle, targetAngle, bank;
    u8 pad006[6];
    f32 x, y, z;
    u8 pad018[12];
    f32 smoothX, smoothY;
} O9Result;
typedef struct O9State {
    s8 mode; u8 p001[3]; f32 input; u8 p008[12];
    f32 axisX, axisY, axisZ; u8 p020[188];
    s16 angle; u8 p0de[10]; f32 tilt, speed; s16 angleTarget;
    u8 p0f2[16]; s16 direction; u8 p104[4]; s16 speedScale;
    u8 p10a[790]; s32 flags;
} O9State;
typedef struct O9Owner {
    u8 p000[4]; s16 bankLimit; u8 p006[6];
    f32 x, y, z; u8 p018[76]; O9State *state;
} O9Owner;

extern u8 D_388[];
extern f32 D_2D0, D_300[], D_340[], D_58, D_70, D_78, D_7C;
extern s16 D_380[];
extern void ext_o0_210b4(f32, s32);
extern s32 ext_o0_214c8(void);
extern s32 ext_o0_2a5bc(s32, s32);
extern f32 ext_o0_2a470(s32);
extern f32 ext_o0_2a46c(s32);

#ifdef NON_MATCHING
void func_overlay_009_F00010B4_186772C(O9Result *out, O9Owner *owner,
                                       f32 stepsFloat) {
    O9State *state = owner->state;
    s32 mode = state->mode & 3;
    s32 steps;
    s32 i;
    s32 tableIndex;
    s16 targetAngle;
    f32 baseX, baseY, baseZ;
    f32 crossA, dot, crossB;
    f32 trigA, trigB, yawA, yawB;
    f32 targetX, targetY;
    f32 cross, targetTilt, speedTarget, blend;

    ext_o0_210b4(60.0f, 0);
    if (state->flags & 8) D_388[mode]++;
    D_388[mode] &= 3;
    tableIndex = D_388[mode] + ((ext_o0_214c8() & 3) * 4);
    targetX = D_300[tableIndex] + (D_2D0 * 75.0f);
    targetY = D_340[tableIndex];
    steps = (s32) stepsFloat;
    targetAngle = D_380[mode];
    i = steps - 1;

    if (steps != 0) {
        do {
            state->angle += ext_o0_2a5bc(state->angle,
                                         0x8000 - state->angleTarget) >> 4;
        } while (i--);
        i = steps - 1;
    }
    if (steps != 0) {
        do {
            out->targetAngle += ext_o0_2a5bc(out->targetAngle,
                                             targetAngle) >> 4;
        } while (i--);
        i = steps - 1;
    }
    if (steps != 0) {
        blend = D_58;
        do {
            out->smoothX += (targetX - out->smoothX) * blend;
            out->smoothY += (targetY - out->smoothY) * blend;
        } while (i--);
        i = steps - 1;
    }

    trigA = ext_o0_2a470(0x8000 - state->angle);
    trigB = ext_o0_2a46c(0x8000 - state->angle);
    yawA = ext_o0_2a470(out->targetAngle - targetAngle);
    yawB = ext_o0_2a46c(out->targetAngle - targetAngle);
    cross = (out->smoothX * yawB) - (out->smoothY * yawA);
    crossA = cross * trigA;
    dot = (out->smoothX * yawA) + (out->smoothY * yawB);
    crossB = cross * trigB;

    if (state->direction == 0) targetTilt = -10.0f;
    else targetTilt = 10.0f;
    if (steps != 0) {
        blend = D_70;
        do {
            state->tilt += (targetTilt - state->tilt) * blend;
        } while (i--);
        i = steps - 1;
    }

    baseX = owner->x + (state->axisX * state->tilt);
    baseY = owner->y + (state->axisY * state->tilt);
    baseZ = owner->z + (state->axisZ * state->tilt);
    trigA = ext_o0_2a470(state->angle + 0x4000);
    trigB = ext_o0_2a46c(state->angle + 0x4000);
    blend = D_78;
    speedTarget = state->input;
    if (speedTarget < 0.0f) speedTarget = -speedTarget;
    if (speedTarget > 1.0f) speedTarget = 1.0f;
    speedTarget *= (f32) state->speedScale * D_7C;
    if (steps != 0) {
        do {
            state->speed += (speedTarget - state->speed) * blend;
        } while (i--);
        i = steps - 1;
    }

    baseX += state->speed * trigA;
    baseZ -= state->speed * trigB;
    out->x = baseX + crossA;
    out->y = baseY + dot;
    out->z = baseZ + crossB;
    out->angle = state->angle;
    if (steps != 0) {
        do {
            out->bank += ext_o0_2a5bc(out->bank,
                                      owner->bankLimit >> 1) >> 5;
        } while (i--);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o009/overlay9UpdateMotion/func_overlay_009_F00010B4_186772C.s")
#endif
