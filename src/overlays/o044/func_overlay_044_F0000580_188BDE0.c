#include "PR/ultratypes.h"

typedef struct Overlay44Gfx {
    u32 w0;
    u32 w1;
} Overlay44Gfx;

typedef struct Overlay44FrameSource {
    s16 dimension0;
    s16 dimension1;
    s16 frameCount;
    u8 storageMode;
    u8 speed;
    u8 *data;
    s32 frameSize;
} Overlay44FrameSource;

typedef struct Overlay44AnimationState {
    s8 sourceIndex;
    u8 mode;
    u8 flags;
    u8 subtype;
    s32 phase;
    s16 value8;
    s16 valueA;
    u8 pad0C[2];
    s8 protectedSlot0;
    s8 protectedSlot1;
    s8 cachedFrame[4];
    void *handles[4];
} Overlay44AnimationState;

#define OVERLAY44_CMD(pkt, a, b)         \
    {                                    \
        Overlay44Gfx *_g = (pkt);        \
        _g->w0 = (u32)(a);               \
        _g->w1 = (u32)(b);               \
    }

extern Overlay44FrameSource *gOverlay44FrameSources;
extern u8 D_0[];
extern u8 D_28[];
extern void func_overlay_044_F0000000_188B860();

/*
 * PLATEAU (2026-08-25): the stock -O2 -mips2 group is best at 347 target-
 * range instructions versus 349, with 330 differing positional words and
 * first mismatch at +0x0. The target's 0x100-byte frame becomes 0x138 because
 * IDO assigns distinct stack homes to the reconstructed display-list macro
 * temporaries; flattening them fixes the frame pressure but loses the target's
 * saved-register topology. No external donor body was used.
 */
#ifdef NON_MATCHING
void func_overlay_044_F0000580_188BDE0(
    Overlay44AnimationState *arg0,
    Overlay44Gfx **arg1,
    f32 arg2) {
    s32 spFC;
    s32 spE4;
    s32 sp64;
    s32 sp58;
    s32 sp54;
    s32 sp50;
    s32 sp4C;
    s32 sp48;
    s32 sp44;
    s32 sp40;
    u8 *var_s0;
    u8 *var_s1;
    f32 temp_f12;
    s32 temp_t4;
    s32 var_t2;
    s32 var_t3;
    s32 temp_a2;
    s32 temp_lo;
    s32 temp_t0;
    s32 temp_t4_2;
    s32 temp_t6;
    s32 temp_v0;
    s32 temp_v1;
    s32 var_a3;
    s32 var_a3_2;
    s32 var_t5;
    u8 temp_t7;
    Overlay44AnimationState *state;
    Overlay44FrameSource *source;

    state = arg0;
    if (state != 0) {
        if (state->sourceIndex != -1) {
            source = &gOverlay44FrameSources[state->sourceIndex];
            var_s0 = state->handles[state->protectedSlot0];
            temp_t4 = source->dimension0;
            var_t3 = source->dimension1;
            var_s1 = state->handles[state->protectedSlot1];
            if (arg2 == 1.0f) {
                OVERLAY44_CMD((*arg1)++, 0x06000000, D_0);
            } else {
                OVERLAY44_CMD((*arg1)++, 0x06000000, D_28);
            }

            temp_t7 = state->subtype;
            temp_v1 = state->phase & 0xFF;
            OVERLAY44_CMD((*arg1)++, 0xFA000000,
                (temp_t7 << 24) | (temp_t7 << 16) |
                (temp_t7 << 8) | 0xFF);
            OVERLAY44_CMD((*arg1)++, 0xFB000000,
                (temp_v1 << 24) | (temp_v1 << 16) |
                (temp_v1 << 8) | temp_v1);

            temp_t6 = state->value8 * 4;
            var_a3 = (s32)(1024.0f / arg2);
            var_t5 = state->valueA << 16;
            temp_f12 = arg2 * 65536.0f;
            spE4 = var_t5;
            spFC = temp_t4;
            temp_a2 = (s32)((f32)temp_t4 * arg2 * 4.0f) + temp_t6;
            if (var_t3 != 0) {
                temp_v0 = temp_t4 * 2;
                sp64 = temp_v0;
                temp_t4_2 = (0x800 / temp_v0) & ~1;
                sp58 = ((((temp_v0 + 7) >> 3) & 0x1FF) << 9) |
                       0xF5100000;
                sp54 = sp58 | 0x100;
                sp50 = (((temp_t4 - 1) * 4) & 0xFFF) << 12;
                sp4C = sp50 | 0x01000000;
                sp48 = ((temp_a2 & 0xFFF) << 12) | 0xE4000000;
                sp44 = (temp_t6 & 0xFFF) << 12;
                temp_v1 = var_a3 & 0xFFFF;
                sp40 = (temp_v1 << 16) | temp_v1;

                do {
                    var_t2 = var_t3;
                    if (temp_t4_2 < var_t3) {
                        var_t2 = temp_t4_2;
                        var_t3 -= temp_t4_2;
                    } else {
                        var_t3 = 0;
                    }

                    OVERLAY44_CMD((*arg1)++, 0xFD100000, var_s1);
                    OVERLAY44_CMD((*arg1)++, 0xF5100100, 0x07080200);
                    var_t5 += (s32)((f32)var_t2 * temp_f12);
                    OVERLAY44_CMD((*arg1)++, 0xE6000000, 0);
                    temp_t0 = (spFC * var_t2) - 1;
                    var_a3_2 = 0x7FF;
                    if (temp_t0 < 0x7FF) {
                        var_a3_2 = temp_t0;
                    }
                    OVERLAY44_CMD((*arg1)++, 0xF3000000,
                        ((var_a3_2 & 0xFFF) << 12) | 0x07000000);
                    OVERLAY44_CMD((*arg1)++, 0xE7000000, 0);
                    OVERLAY44_CMD((*arg1)++, sp54, 0x01080200);
                    OVERLAY44_CMD((*arg1)++, 0xF2000000,
                        sp4C | (((var_t2 - 1) * 4) & 0xFFF));

                    OVERLAY44_CMD((*arg1)++, 0xFD100000, var_s0);
                    OVERLAY44_CMD((*arg1)++, 0xF5100000, 0x07080200);
                    OVERLAY44_CMD((*arg1)++, 0xE6000000, 0);
                    var_a3 = 0x7FF;
                    if (temp_t0 < 0x7FF) {
                        var_a3 = temp_t0;
                    }
                    OVERLAY44_CMD((*arg1)++, 0xF3000000,
                        ((var_a3 & 0xFFF) << 12) | 0x07000000);
                    OVERLAY44_CMD((*arg1)++, 0xE7000000, 0);
                    OVERLAY44_CMD((*arg1)++, sp58, 0x00080200);
                    OVERLAY44_CMD((*arg1)++, 0xF2000000,
                        sp50 | (((var_t2 - 1) * 4) & 0xFFF));

                    OVERLAY44_CMD((*arg1)++,
                        sp48 | ((var_t5 >> 14) & 0xFFF),
                        sp44 | ((spE4 >> 14) & 0xFFF));
                    OVERLAY44_CMD((*arg1)++, 0xB3000000, 0);
                    OVERLAY44_CMD((*arg1)++, 0xB2000000, sp40);

                    spE4 = var_t5;
                    temp_lo = var_t2 * sp64;
                    var_s0 += temp_lo;
                    var_s1 += temp_lo;
                } while (var_t3 != 0);
            }

            func_overlay_044_F0000000_188B860(arg1);
            OVERLAY44_CMD((*arg1)++, 0xFA000000, 0xFFFFFFFF);
            OVERLAY44_CMD((*arg1)++, 0xFB000000, 0xFFFFFFFF);
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o044/func_overlay_044_F0000580_188BDE0/func_overlay_044_F0000580_188BDE0.s")
#endif
