#include "PR/ultratypes.h"

typedef struct O9Control {
    u8 pad000[4];
    f32 lean;
    u8 pad008[0xE8];
    s16 angle;
    u8 pad0F2[0x16];
    s16 angleStep;
    u8 pad10A[0x312];
    u32 flags;
    u8 pad420[8];
    s32 inputY;
    s32 inputX;
} O9Control;

typedef struct O9State {
    f32 x;
    f32 y;
    f32 throttle;
    f32 position;
    f32 acceleration;
    f32 turnRate;
} O9State;

extern f32 D_4C;

#ifdef NON_MATCHING
void func_overlay_009_F00009BC_1867034(s16 *angleOut, O9Control *control,
                                       O9State *state) {
    f32 target;
    f32 one = 1.0f;
    s32 input;

    input = control->inputX;
    if (input < -59) {
        target = -1.0f;
    } else if (input >= 60) {
        target = one;
    } else {
        target = (f32)input / 60.0f;
    }
    state->x += (target - state->x) * 0.02f;
    if (state->x > 0.0f) {
        control->lean = -state->x * 20.0f;
    } else {
        control->lean = -state->x * 10.0f;
    }

    input = control->inputY;
    if (input < -59) {
        target = -1.0f;
    } else if (input >= 60) {
        target = one;
    } else {
        target = (f32)input / 60.0f;
    }
    state->y += (-target - state->y) * 0.075f;

    if (control->flags & 0x10) {
        state->throttle += (2.0f - state->throttle) * 0.025f;
    } else {
        state->throttle += (one - state->throttle) * 0.05f;
    }

    state->turnRate +=
        ((state->throttle * state->y * 365.0f) - state->turnRate) * 0.1f;
    control->angleStep = (s16)(s32)state->turnRate;
    control->angle += control->angleStep;
    *angleOut = control->angle;

    if (control->flags & 0x8000) {
        state->acceleration += 0.1f;
        if (state->acceleration > 3.0f) {
            state->acceleration = 3.0f;
        }
    } else if (control->flags & 0x4000) {
        state->acceleration *= 0.95f;
        if ((state->acceleration > -0.01f) &&
            (state->acceleration < 0.01f)) {
            state->acceleration = 0.0f;
        }
    } else {
        state->acceleration -= 0.1f;
    }

    state->position += state->acceleration;
    if (state->position < 40.0f) {
        state->position = (40.0f - state->position) + 40.0f;
        state->acceleration *= -0.4f;
        if ((state->acceleration > -0.1f) &&
            (state->acceleration < D_4C)) {
            state->acceleration = 0.0f;
            state->position = 40.0f;
            return;
        }
    } else if (state->position > 200.0f) {
        state->position = 200.0f;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o009/overlay9UpdateInputState/func_overlay_009_F00009BC_1867034.s")
#endif
