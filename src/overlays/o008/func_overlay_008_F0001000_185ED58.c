#include "PR/ultratypes.h"

typedef struct O8PhaseState {
    u8 pad000[0x16A];
    s16 timer;
    u8 pad16C[0x18];
    u8 forceEffect;
    u8 phase;
    u8 effect;
    u8 countdown;
    f32 weight;
    u8 pad18C[0x290];
    u32 flags;
} O8PhaseState;

extern u8 gO8RolloverControlReloc;
extern u8 gO8Phase2ScaleControlReloc;
extern u8 gO8Phase3ScaleControlReloc;
extern const f32 gO8Phase2TargetReloc;
extern const f32 gO8Phase2ScaleReloc;
extern const f32 gO8Phase3DecayReloc;
extern const f32 gO8RetireThresholdReloc;
extern const f32 gO8Phase3ScaleReloc;

extern f32 o8RolloverSampleReloc(void);
extern void o8Phase1EmitReloc(O8PhaseState *state, s32 kind, f32 scale);

f32 func_overlay_008_F0001000_185ED58(void *unused, O8PhaseState *state, f32 input) {
    s32 nextCountdown;

    if (state->timer > 0) {
        if ((state->phase != 1) && (state->phase != 2)) {
            state->phase = 2;
            state->countdown = 0;
        }
    }

    switch (state->phase) {
    case 1:
        state->weight += (1.0f - state->weight) * 0.875f;
        state->flags |= 0x8000;
        state->effect = 3;
        state->countdown--;

        if (state->countdown == 0) {
            nextCountdown = 120;
            if (gO8RolloverControlReloc != 0) {
                nextCountdown =
                    (s32)(o8RolloverSampleReloc() * 6.0f + 60.0f);
                if (nextCountdown >= 181) {
                    nextCountdown = 180;
                }
            }
            state->phase = 2;
            state->countdown = (u8)nextCountdown;
            state->weight = 1.0f;
        }

        o8Phase1EmitReloc(state, 75, 0.5f);
        break;

    case 2:
        state->effect = 3;
        if (state->countdown != 0) {
            state->countdown--;
            state->flags |= 0x8000;
        } else if (state->timer > 0) {
            state->weight +=
                (gO8Phase2TargetReloc - state->weight) * 0.875f;
        } else {
            state->phase = 3;
        }

        if (gO8Phase2ScaleControlReloc == 0) {
            input += gO8Phase2ScaleReloc * state->weight;
        } else {
            input += 7.0f * state->weight;
        }
        break;

    case 3:
        state->effect = 0;
        state->weight *= gO8Phase3DecayReloc;
        if (state->weight < gO8RetireThresholdReloc) {
            state->phase = 0;
            state->weight = 0.0f;
        }

        if (gO8Phase3ScaleControlReloc == 0) {
            input += gO8Phase3ScaleReloc * state->weight;
        } else {
            input += 7.0f * state->weight;
        }
        break;

    default:
        state->effect = 0;
        break;
    }

    if (state->forceEffect != 0) {
        state->effect |= 3;
    }
    return input;
}
