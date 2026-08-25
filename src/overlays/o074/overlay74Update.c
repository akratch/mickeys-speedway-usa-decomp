#include "PR/ultratypes.h"

typedef struct Overlay74UpdateState {
    u8 strength;
    u8 channel;
    s8 minimum;
    s8 maximum;
} Overlay74UpdateState;

typedef struct Overlay74HitState {
    s8 strength;
} Overlay74HitState;

typedef struct Overlay74UpdateObject {
    s16 angle;
    u8 pad02[4];
    s16 flags;
    u8 pad08[4];
    f32 x;
    f32 y;
    s32 z;
    u8 pad18[0x4C];
    Overlay74UpdateState *state;
} Overlay74UpdateObject;

typedef struct Overlay74QueryResult {
    Overlay74UpdateObject *object;
    u8 pad04[0x30];
} Overlay74QueryResult;

extern u32 gOverlay74Flags;

Overlay74UpdateObject *overlay74QueryReloc(f32 x, f32 y, s32 z, f32 strength,
                                           s32 enabled,
                                           Overlay74QueryResult *result);
void overlay74HitReloc(Overlay74UpdateObject *object);
void overlay74SoundReloc(s32 soundId, s32 arg1);
void overlay74RewardReloc(s32 count);

/* NON_MATCHING plateau (retested 2026-08-25): the nearest skeleton score is
 * 0.056 and all 119 flag combinations miss. Ten structural variants reduced
 * the exact-size, 100-word candidate from 14 differing words to six while
 * preserving the 0x60 frame. The first mismatch remains +0xC: five words swap
 * the result aggregate's address/object register pair, and one later OR uses
 * the opposite commutative encoding. A bounded two-worker permuter batch found
 * no exact form. */
#ifdef NON_MATCHING
void overlay74Update(Overlay74UpdateObject *object, s32 amount) {
    Overlay74QueryResult result;
    f32 delta;
    Overlay74UpdateState *state;
    s32 mask;

    if (!(object->flags & 0x400)) {
        object->angle += amount << 8;
        state = object->state;
        /* Preserve IDO's original aggregate-address allocation. */
        if (!&result) {
        }
        if (overlay74QueryReloc(object->x, object->y, object->z,
                                (f32)state->strength, 1, &result)) {
            amount = (s32)result.object->state;
            delta = result.object->y - object->y;
            if ((((Overlay74HitState *)amount)->strength == 0) &&
                ((f32)state->minimum < delta) &&
                (delta < (f32)state->maximum)) {
                object->flags |= 0x400;
                *(u16 *)&gOverlay74Flags =
                    (*(u16 *)&gOverlay74Flags & 0xF87F) |
                    ((((((gOverlay74Flags << 5) >> 28) |
                        (1 << state->channel)) << 1) << 6) & 0x780);
                overlay74HitReloc(object);
                overlay74SoundReloc(0x27C, 0);

                object = (Overlay74UpdateObject *)5;
                if (!object) {
                }
                state = (Overlay74UpdateState *)((gOverlay74Flags << 5) >> 28);
                mask = 8;
                do {
                    if ((s32)state & mask) {
                        object = (Overlay74UpdateObject *)((s32)object + 1);
                    }
                    mask >>= 1;
                } while (mask != 0);
                if ((s32)object >= 6) {
                    overlay74RewardReloc((s32)object);
                }
            }
        }
    }
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o074/overlay74Update/func_overlay_074_F00000B8_18CBD58.s")
#endif
