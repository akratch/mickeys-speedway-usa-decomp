#include "PR/ultratypes.h"

typedef struct Overlay36Inner {
    s8 selector;
    u8 pad001[0x19A];
    u8 countdown;
    u8 pad19C[0x1E9];
    u8 strength;
} Overlay36Inner;

typedef struct Overlay36Object {
    u8 pad000[0x64];
    Overlay36Inner *inner;
} Overlay36Object;

typedef struct Overlay36Choice {
    u8 pad0[3];
    u8 value;
} Overlay36Choice;

typedef struct Overlay36Node {
    u8 pad0[4];
    s16 value4;
    s16 flags6;
} Overlay36Node;

extern s32 gOverlay36Mode;
extern u8 gOverlay36AdjustEnabled;
extern u16 gOverlay36EnabledMask;
extern u8 gOverlay36Weights[14][10];
extern u8 gOverlay36AltTable[];
extern f32 gOverlay36ScaleDivisor;
extern f32 gOverlay36ScaleMultiplier;
extern f32 gOverlay36StrengthMultiplier;
extern Overlay36Node *gOverlay36NodeA;
extern Overlay36Node *gOverlay36NodeB;

extern u32 overlay36ChooseReloc();
extern f32 overlay36MeasureReloc(Overlay36Object *object, s32 mode);
extern Overlay36Choice *overlay36GetChoiceReloc(s8 selector, s32 arg1);
extern void func_overlay_036_F0000914_1883DCC(Overlay36Object *object,
                                              s32 arg1, s32 state,
                                              s32 enabled);

/* Workbench p6 batch 13: structure-mismatch; 170 instructions/-0x38 frame, 71-word masked floor, first +0x3C.
 * Lever: swapping the final blend operands preserved shape and reduced the prior 155-word floor by 84 words.
 * Remains: early mode-branch call staging, FP-pool webs, and 34 relocation identities. */
#ifdef NON_MATCHING
void func_overlay_036_F0000A60_1883F18(Overlay36Object *object, s32 arg1,
                                       volatile s32 arg2,
                                       volatile s32 arg3) {
    Overlay36Inner *inner;
    s32 total;
    s32 state;
    s32 position;
    s32 i;
    f32 value;
    f32 blend;
#if BLEND_SHAPE == 1
    f32 blendedBase;
#elif BLEND_SHAPE == 2
    f32 blendedRemainder;
#endif

    inner = object->inner;
    total = 0;
    if (inner->countdown == 0) {
        if (gOverlay36Mode == 3) {
            state = gOverlay36AltTable[overlay36ChooseReloc(0, 5)];
        } else {
            value = (overlay36MeasureReloc(object, 5) /
                     gOverlay36ScaleDivisor) * gOverlay36ScaleMultiplier;
            value += (f32)inner->strength * gOverlay36StrengthMultiplier;

            if (gOverlay36AdjustEnabled != 0) {
                Overlay36Choice *choice;

                choice = overlay36GetChoiceReloc(inner->selector, 0);
                if (choice->value >= 0x21) {
                    blend = 32.0f;
                } else {
                    blend = (f32)choice->value;
                }
                blend *= 0.015625f;
#if BLEND_SHAPE == 1
                blendedBase = blend * 10.0f;
                value = blendedBase + ((1.0f - blend) * value);
#elif BLEND_SHAPE == 2
                blendedRemainder = (1.0f - blend) * value;
                value = (blend * 10.0f) + blendedRemainder;
#else
                value = ((1.0f - blend) * value) + (blend * 10.0f);
#endif
            }

            state = -1;
            position = (s32)value;
            if (position >= 10) {
                position = 9;
            }

            i = 13;
            do {
                if (gOverlay36EnabledMask & (1 << i)) {
                    state = i;
                    total += gOverlay36Weights[i][position];
                }
            } while (i--);

            if (total >= 2) {
                state = overlay36ChooseReloc(1, total, state, position);
                i = 13;
                do {
                    if (gOverlay36EnabledMask & (1 << i)) {
                        state -= gOverlay36Weights[i][position];
                        if (state <= 0) {
                            state = i;
                            break;
                        }
                    }
                } while (i--);
            }

            if (state == -1) {
                state = 9;
            }
        }
        func_overlay_036_F0000914_1883DCC(object, arg1, state, 1);
    }

    if (gOverlay36Mode == 3) {
        gOverlay36NodeA->value4 = 0xF0;
    } else {
        gOverlay36NodeA->value4 = 0x3C;
    }
    gOverlay36NodeB->flags6 |= 0x400;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o036/overlay36ChooseWeightedState/func_overlay_036_F0000A60_1883F18.s")
#endif
