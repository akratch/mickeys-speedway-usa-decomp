#include "PR/ultratypes.h"

typedef struct Overlay41Descriptor {
    u8 reserved00[0x14];
    u16 flags14;
    s8 reserved16[0xBE];
    f32 scaleD4;
} Overlay41Descriptor;

typedef struct Overlay41State {
    u8 reserved00[6];
    s16 flags06;
    u8 reserved08[0x31];
    u8 value39;
    u8 reserved3A[6];
    Overlay41Descriptor *descriptor40;
    u8 reserved44[0x4F];
    u8 selector93;
} Overlay41State;

typedef struct Overlay41Input {
    u8 reserved00[0x0C];
    f32 amount0C;
    u8 reserved10[4];
    u8 negate14;
    u8 reserved15;
    u8 flags16;
    u8 reserved17[0x0D];
    u8 start24;
    u8 end25;
    u8 limit26;
    u8 current27;
} Overlay41Input;

extern Overlay41State *gOverlay41CurrentState;
extern void overlay41ApplyAmount(Overlay41State *state, f32 amount, f32 step);

/* Workbench: mixed(structural:2, schedule:2, register:25), exact 115 instructions/34 words, first +0xE8.
 * Levers: start/divisor lifetime and u8/s32/block-local/declaration-order forms; all regressed.
 * Remains: limit-to-a0/start-to-v1 pool routing and the tail temporary phase. */
#ifdef NON_MATCHING
void func_overlay_041_F0001298_18885D0(Overlay41Input *input,
                                        Overlay41State *state, s32 step) {
    Overlay41Descriptor *descriptor;
    s32 active;
    s32 value;
    s32 current;
    s32 limit;
    s32 divisor;
    u8 start;

    descriptor = state->descriptor40;
    active = descriptor->scaleD4 != 0.0f;
    if ((active && descriptor->reserved16[state->selector93 + 8] == 0) ||
        (!active && descriptor->reserved16[8] == 0)) {
        if (input->negate14 != 0) {
            overlay41ApplyAmount(state, -input->amount0C, (f32)step);
        } else {
            overlay41ApplyAmount(state, input->amount0C, (f32)step);
        }
    }

    current = input->current27;
    limit = input->limit26;
    if (current < limit) {
        divisor = limit;
        input->current27 = current + step;
        current = input->current27;
        if (current >= limit) {
            input->current27 = limit;
            current = limit & 0xFF;
        }
        start = input->start24;
        value = start + ((input->end25 - start) * current) / divisor;
        state->value39 = value;
        if ((u8)value < 0xFF) {
            state->flags06 |= 4;
        } else if (!(state->descriptor40->flags14 & 4)) {
            state->flags06 &= ~4;
        }
    } else {
        state->value39 = input->end25;
    }

    if (input->flags16 & 2) {
        gOverlay41CurrentState = state;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o041/overlay41UpdateProgress/func_overlay_041_F0001298_18885D0.s")
#endif
