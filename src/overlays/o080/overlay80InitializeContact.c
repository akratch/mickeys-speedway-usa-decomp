#include "PR/ultratypes.h"

typedef struct Overlay80State {
    f32 previousValue;
    f32 initializedValue;
    s32 active;
} Overlay80State;

typedef struct Overlay80Notice {
    u8 pad00[4];
    u8 state;
} Overlay80Notice;

typedef struct Overlay80Candidate {
    f32 threshold;
    s32 value;
} Overlay80Candidate;

typedef struct Overlay80Object {
    s16 key;
    s16 outputB;
    s16 outputA;
    u8 pad06[2];
    f32 scaledValue;
    f32 position0;
    f32 limit;
    f32 position1;
    u8 pad18[0x28];
    f32 *scale;
    u8 pad44[0x0C];
    Overlay80Notice *notice;
    u8 pad54[0x10];
    Overlay80State *state;
} Overlay80Object;

typedef struct Overlay80Init {
    u8 pad00[0x0A];
    s16 key;
    s16 scale;
    s16 initialValue;
} Overlay80Init;

extern f32 gOverlay80Scale0;
extern s32 overlay80QueryNearbyReloc(
    f32 position0, f32 position1, s32 radius,
    Overlay80Candidate ***candidatesOut);
extern void overlay80ResolveCandidateReloc(
    s16 key, s32 selectedValue, s16 *outputA, s16 *outputB);

/*
 * Pinned DKR v77/v80 and JFG object scans found no exact donor.
 * Plateau (2026-08-25, 5 source forms plus a bounded permuter batch):
 * -O2 -mips2 -Wab,-r4300_mul has the exact 71-word size and differs in
 * one word at +0x4C. IDO commutes the outer floating-point multiply; explicit
 * temporaries and a volatile-qualified access disturb the surrounding
 * allocation, while the permuter found no candidate below its base score.
 */
#ifdef NON_MATCHING
void overlay80InitializeContact(Overlay80Object *object,
                                const Overlay80Init *init) {
    Overlay80State *state;
    Overlay80Candidate **candidates;
    s32 count;
    s32 index;
    s32 selected;

    state = object->state;
    object->scaledValue =
        (f32)init->scale * gOverlay80Scale0 * *object->scale;
    object->key = init->key;

    count = overlay80QueryNearbyReloc(object->position0, object->position1,
                                      0x1800, &candidates);
    if (count != 0) {
        selected = -1;
        for (index = 0; index < count; index++) {
            if (candidates[index]->threshold <= object->limit) {
                selected = index;
                break;
            }
        }

        if (selected != -1) {
            overlay80ResolveCandidateReloc(
                object->key, candidates[index]->value, &object->outputA,
                &object->outputB);
        }
    }

    state->initializedValue = (f32)init->initialValue;
    if (object->notice != 0) {
        object->notice->state = 2;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o080/overlay80InitializeContact/func_overlay_080_F0000000_18CE8C8.s")
#endif
