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

typedef struct Overlay36AltTable {
    u8 pad0[0xA8];
    u8 values[1];
} Overlay36AltTable;

typedef struct Overlay36ScaleDivisorData {
    u8 pad0[0x28];
    f32 value;
} Overlay36ScaleDivisorData;

typedef struct Overlay36ScaleMultiplierData {
    u8 pad0[0x2C];
    f32 value;
} Overlay36ScaleMultiplierData;

typedef struct Overlay36StrengthMultiplierData {
    u8 pad0[0x30];
    f32 value;
} Overlay36StrengthMultiplierData;

typedef struct Overlay36NodeDataA {
    u8 pad0[0x150];
    Overlay36Node *node;
} Overlay36NodeDataA;

typedef struct Overlay36NodeDataB {
    u8 pad0[0x14C];
    Overlay36Node *node;
} Overlay36NodeDataB;

extern s32 gOverlay36Mode;
extern u8 gOverlay36AdjustEnabled;
extern u16 gOverlay36EnabledMask;
extern u8 gOverlay36Weights[14][10];
extern Overlay36AltTable gOverlay36AltTable;
extern Overlay36ScaleDivisorData gOverlay36ScaleDivisor;
extern Overlay36ScaleMultiplierData gOverlay36ScaleMultiplier;
extern Overlay36StrengthMultiplierData gOverlay36StrengthMultiplier;
extern Overlay36NodeDataA gOverlay36NodeA;
extern Overlay36NodeDataB gOverlay36NodeB;

extern u32 overlay36ChooseReloc();
extern f32 overlay36MeasureReloc(Overlay36Object *object, s32 mode);
extern Overlay36Choice *overlay36GetChoiceReloc(s8 selector, s32 arg1);
extern void func_overlay_036_F0000914_1883DCC(Overlay36Object *object,
                                              s32 arg1, s32 state,
                                              s32 enabled);

/* Workbench p5: mixed (constant:2, structural:14, schedule:2), 170/170 instructions/frame -56, 155 masked words, first +0x3C.
 * Aggregate data layouts fixed the table, scalar, and node addends; combined FP association fixed the prior FP-web residual.
 * Remaining: mode-branch and loop scheduling, with relocation identities still unresolved. */
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
            state = gOverlay36AltTable.values[overlay36ChooseReloc(0, 5)];
        } else {
            value = ((overlay36MeasureReloc(object, 5) /
                      gOverlay36ScaleDivisor.value) *
                         gOverlay36ScaleMultiplier.value) +
                    ((f32)inner->strength * gOverlay36StrengthMultiplier.value);

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
                value = (blend * 10.0f) + ((1.0f - blend) * value);
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
        gOverlay36NodeA.node->value4 = 0xF0;
    } else {
        gOverlay36NodeA.node->value4 = 0x3C;
    }
    gOverlay36NodeB.node->flags6 |= 0x400;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o036/overlay36ChooseWeightedState/func_overlay_036_F0000A60_1883F18.s")
#endif
