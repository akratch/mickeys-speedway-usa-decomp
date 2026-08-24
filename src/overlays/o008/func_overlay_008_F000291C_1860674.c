#include "PR/ultratypes.h"

typedef struct O8P291CLink {
    u8 pad00[0x62];
    u8 gate62;
} O8P291CLink;

typedef struct O8P291CMotion {
    s16 heading0;
    u8 pad02[0x0A];
    f32 positionC;
    f32 position10;
    f32 position14;
    u8 pad18[4];
    f32 velocity1C;
    f32 velocity20;
    f32 velocity24;
    u8 pad28[6];
    s16 state2E;
    u8 pad30[0x18];
    O8P291CLink *link48;
} O8P291CMotion;

typedef struct O8P291CState {
    u8 pad00[4];
    f32 control4;
    f32 control8;
    f32 blendC;
    u8 pad10[0x28];
    f32 origin38;
    f32 origin3C;
    f32 origin40;
    u8 pad44[0x30];
    f32 axis74;
    f32 axis78;
    f32 axis7C;
    f32 speed80;
    f32 speed84;
    f32 accel88;
    u8 pad8C[8];
    f32 delta94;
    f32 delta98;
    f32 delta9C;
    u8 padA0[0x50];
    s16 angleF0;
    u8 padF2[0x0A];
    s16 angleFC;
    s16 angleFE;
    u8 pad100[4];
    s16 angle104;
    u8 pad106[0x64];
    s16 mode16A;
    u8 pad16C[4];
    u8 reset170;
    u8 pad171[0x10];
    u8 active181;
    u8 pad182[3];
    u8 suppress185;
    u8 pad186[0x296];
    u32 flags41C;
    u8 pad420[0x18];
    s32 mode438;
} O8P291CState;

typedef struct O8P291CBlendView {
    u8 pad00[0x0C];
    f32 blendC;
} O8P291CBlendView;

extern f32 O8P291C_data_1A0;
extern f32 O8P291C_data_1A4;
extern f32 O8P291C_data_1A8;
extern f32 O8P291C_data_1AC;
extern f32 O8P291C_gravity;
extern f32 D_10;

extern f32 func_overlay_008_F0000000_185DD58(f32 value, s32 updateRate);
extern f32 O8P291C_call_sin(s16 angle);
extern f32 O8P291C_call_cos(s16 angle);
extern void func_overlay_008_F0004CF0_1862A48(
    O8P291CMotion *motion, O8P291CState *state, s32 updateRate);
extern s32 O8P291C_call_037C(O8P291CMotion *motion, O8P291CState *state,
                             f32 update);
extern s32 O8P291C_call_039C(O8P291CMotion *motion, f32 x, f32 y, f32 z);

#ifdef NON_MATCHING
void func_overlay_008_F000291C_1860674(O8P291CMotion *motion,
                                       O8P291CState *state,
                                       f32 update) {
    s16 angle;
    s32 savedRate;
    f32 horizontal;
    f32 vertical;
    f32 invUpdate;
    s32 contact;
    f32 targetBlend;
    f32 desiredBlend;
    f32 displacementX;
    f32 displacementY;
    f32 displacementZ;
    f32 scale;

    motion->heading0 = state->angleF0 + state->angleFC + state->angle104;
    angle = state->angleF0 + state->angleFE;

    if (state->mode438 == 1) {
        f32 attenuation = func_overlay_008_F0000000_185DD58(O8P291C_data_1A0, (s32)update);
        if ((state->control4 < -0.5f) || (state->control4 > 0.5f)) {
            state->control4 *= attenuation;
        } else {
            state->control4 = 0.0f;
        }
        if ((state->control8 < -0.5f) || (state->control8 > 0.5f)) {
            state->control8 *= attenuation;
        } else {
            state->control8 = 0.0f;
        }
    }

    savedRate = (s32)update;
    if (state->active181 != 0) {
        f32 distance = state->speed84 * update +
                       (0.5f * state->accel88 * update * update);
        displacementX = state->axis74 * distance;
        displacementY = state->axis78 * distance;
        displacementZ = state->axis7C * distance;
        if (distance < 0.0f) {
            state->speed84 = 0.0f;
            state->accel88 = 0.0f;
            state->active181 = 0;
            if (((state->flags41C & 0x8000) == 0) &&
                (state->suppress185 == 0)) {
                state->control4 = 0.0f;
                state->control8 = 0.0f;
            }
        }
        scale = 1.0f - state->speed84 / state->speed80;
        state->speed84 += state->accel88 * update;
        horizontal = O8P291C_call_sin(angle) * state->control4 * scale;
        vertical = O8P291C_call_cos(angle) * state->control4 * scale;
    } else {
        displacementX = 0.0f;
        displacementY = 0.0f;
        displacementZ = 0.0f;
        horizontal = O8P291C_call_sin(angle) * state->control4;
        vertical = O8P291C_call_cos(angle) * state->control4;
    }

    horizontal += state->control8 * O8P291C_call_cos(angle);
    vertical -= state->control8 * O8P291C_call_sin(angle);
    func_overlay_008_F0004CF0_1862A48(motion, state, savedRate);

    {
        f32 nextY;
        f32 nextX;
        f32 nextZ;
        nextY = (motion->velocity20 * update) -
            (0.5f * O8P291C_gravity * update * update) + displacementY;
        invUpdate = 1.0f / update;
        nextX = horizontal * update + displacementX;
        nextZ = vertical * update + displacementZ;
        motion->velocity1C = nextX * invUpdate;
        motion->velocity20 -= O8P291C_gravity * update;
        motion->velocity24 = nextZ * invUpdate;
        motion->positionC += nextX;
        motion->position10 += nextY;
        motion->position14 += nextZ;
    }

    contact = O8P291C_call_037C(motion, state, update);
    if ((O8P291C_call_039C(motion, 0.0f, 0.0f, 0.0f) != 0) ||
        (motion->state2E == -1)) {
        state->reset170 = 1;
        motion->positionC = state->origin38;
        motion->position10 = state->origin3C;
        motion->position14 = state->origin40;
        O8P291C_call_039C(motion, 0.0f, 0.0f, 0.0f);
    }

    state->delta94 = (motion->positionC - state->origin38) * invUpdate;
    state->delta98 = (motion->position10 - state->origin3C) * invUpdate;
    state->delta9C = (motion->position14 - state->origin40) * invUpdate;

    if ((state->mode16A == 0) &&
        ((contact != 0) || (motion->link48->gate62 != 0))) {
        invUpdate = (state->flags41C & 0x8000) ? 3.0f : 0.0f;
        state->blendC +=
            (invUpdate - state->blendC) *
            (1.0f - func_overlay_008_F0000000_185DD58(O8P291C_data_1A4, savedRate));
        targetBlend = ((volatile O8P291CBlendView *)state)->blendC;
        if (state->control4 < -targetBlend) state->control4 = -targetBlend;
        if (targetBlend < state->control4) state->control4 = targetBlend;
        if (state->control8 < -targetBlend) state->control8 = -targetBlend;
        if (targetBlend < state->control8) state->control8 = targetBlend;
    } else {
        if (D_10 < state->blendC + O8P291C_data_1A8) {
            state->blendC = D_10;
            return;
        }
        state->blendC +=
            (D_10 - state->blendC) *
            (1.0f - func_overlay_008_F0000000_185DD58(O8P291C_data_1AC, savedRate));
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o008/func_overlay_008_F000291C_1860674/func_overlay_008_F000291C_1860674.s")
#endif
