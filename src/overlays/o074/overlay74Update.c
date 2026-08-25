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

/* NON_MATCHING plateau (reconfirmed 2026-08-25): the nearest skeleton score is
 * 0.056 and all 119 flag combinations miss. The exact-size 100-word candidate
 * differs in 14 words, first at +0xC: IDO colors the result aggregate address
 * as v0 rather than retail's t3, reverses the later flags/mask web, and emits
 * one commutative OR encoding oppositely. Natural scalar loop locals reproduce
 * the retail loop registers but enlarge the 0x60 frame to 0x68. A bounded
 * two-worker permuter batch found no exact form; its cached-flags mutation
 * regressed to 18 differing words under the canonical -mips2 build. */
#ifdef NON_MATCHING
void overlay74Update(Overlay74UpdateObject *object, s32 amount) {
    Overlay74QueryResult result;
    f32 delta;
    Overlay74UpdateState *state;

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
                state = (Overlay74UpdateState *)8;
                do {
                    if (((gOverlay74Flags << 5) >> 28) & (s32)state) {
                        object = (Overlay74UpdateObject *)((s32)object + 1);
                    }
                    state = (Overlay74UpdateState *)((s32)state >> 1);
                } while (state != 0);
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
