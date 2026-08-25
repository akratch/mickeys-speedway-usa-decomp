#include "PR/ultratypes.h"

typedef struct Overlay36State {
    u8 pad0[4];
    u16 active;
} Overlay36State;

typedef struct Overlay36Object {
    u8 pad0[6];
    s16 flags;
    u8 pad8[4];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x4C];
    Overlay36State *state;
} Overlay36Object;

typedef struct Overlay36Nearby {
    u8 pad0[0x10];
    f32 y;
} Overlay36Nearby;

typedef struct Overlay36WorldState {
    u8 pad0[0xA];
    u8 changed;
} Overlay36WorldState;

extern s32 overlay36FindNearbyReloc(f32, f32, f32, f32, s32,
                                    Overlay36Nearby **);
extern Overlay36WorldState *gOverlay36WorldStateReloc;

/* Mickey-local reconstruction; pinned DKR v77/v80 are negative and JFG's
 * Overlay 36 hits occur only at the unrelated +0x1470/+0x1490 wrappers. */
/* Plateau after the flag lattice, eight source-layout attempts, and a bounded
 * permuter batch: exact 0xFC size and 50/63 words, first mismatch at +0x0.
 * Reusing one nearby-height load closed most of the prior gap; the remaining
 * mismatch is the 0x80-versus-0x70 frame and FP-bound scheduling/registers.
 * The permuter's lower numerical candidate read center before initialization
 * and was rejected as non-equivalent.
 * Follow-up (2026-08-25): a nine-entry result array reached a 0x70 frame but
 * left the array and spill offsets +0x10 from target; direct bound expressions
 * retained the 0x80 frame and regressed FP scheduling. The first mismatch of
 * the best 50/63-word candidate remains +0x0. */
#ifdef NON_MATCHING
void func_overlay_036_F0000818_1883CD0(Overlay36Object *object,
                                       s32 remaining) {
    Overlay36Nearby *results[13];
    Overlay36State *state;
    Overlay36Nearby **current;
    Overlay36Nearby *nearby;
    s32 i;
    f32 center;
    f32 low;
    f32 high;

    state = object->state;
    if (state->active == 0) {
        remaining = overlay36FindNearbyReloc(object->x, object->y, object->z,
                                              64.0f, 1, results);
        if (remaining != 0) {
            high = 45.0f;
            center = object->y;
            i = remaining - 1;
            low = center - high;
            if (remaining != 0) {
                high += center;
                current = &results[i];
                do {
                    nearby = *current--;
                    center = nearby->y;
                    if ((center < low) || (high < center)) {
                        remaining--;
                    }
                } while (i--);
            }
        }
        if (remaining != 0) {
            state->active = 1;
        } else {
            object->flags &= ~0x400;
            gOverlay36WorldStateReloc->changed = 1;
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o036/overlay36CheckNearbyHeight/func_overlay_036_F0000818_1883CD0.s")
#endif
