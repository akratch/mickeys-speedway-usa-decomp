#include "PR/ultratypes.h"

typedef struct Overlay1SearchState {
    u16 xRange;
    u16 zRange;
    u8 pad04[2];
    u8 flags;
    u8 pad07;
    u8 lookupKey;
    u8 pad09[2];
    u8 active;
    u8 pad0C[0x3A8];
    s16 counter;
} Overlay1SearchState;

typedef struct Overlay1SearchObject {
    u8 pad00[0xC];
    f32 x;
    u8 pad10[4];
    f32 z;
    u8 pad18[0x2C];
    s16 type;
    u8 pad46[0x1E];
    Overlay1SearchState *state;
} Overlay1SearchObject;

extern Overlay1SearchObject **func_8000572C(s32 *first, s32 *limit);
extern Overlay1SearchObject *func_80005820(u8 key);
extern void overlay4RemoveObject(Overlay1SearchObject *object);

#ifdef NON_MATCHING
void overlay1SearchNearby(Overlay1SearchObject *object, void *unused) {
    Overlay1SearchState *range;
    s32 first;
    s32 limit;
    s32 index;
    Overlay1SearchObject **objects;
    Overlay1SearchObject *candidate;
    Overlay1SearchObject *linked;
    Overlay1SearchState *state;
    f32 delta;
    f32 threshold;

    (void)unused;
    range = object->state;
    objects = func_8000572C(&first, &limit);
    index = first;
    if (index < limit) {
        do {
            candidate = objects[index];
            if (candidate->type == 0x21) {
                delta = candidate->x - object->x;
                state = candidate->state;
                threshold = range->xRange;
                if (delta < 0.0f) {
                    delta = -delta;
                }
                if (delta <= threshold) {
                    delta = candidate->z - object->z;
                    threshold = range->zRange;
                    if (delta < 0.0f) {
                        delta = -delta;
                    }
                    if (delta <= threshold) {
                        linked = func_80005820(range->lookupKey);
                        if (linked != 0) {
                            linked->state->counter++;
                        }
                        state->flags |= 4;
                        state->active = 1;
                        overlay4RemoveObject(candidate);
                        return;
                    }
                }
            }
            index++;
        } while (index != limit);
    }
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay1SearchNearby/func_overlay_001_F0006B6C_1852F4C.s")
#endif
