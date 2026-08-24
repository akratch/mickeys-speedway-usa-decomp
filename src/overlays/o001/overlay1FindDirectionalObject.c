#include "PR/ultratypes.h"

#ifndef DOT_CONDITION
#define DOT_CONDITION (threshold < dot)
#endif

typedef struct Overlay1ObjectState { s8 tableIndex; } Overlay1ObjectState;
typedef struct Overlay1Object {
    s16 angle; u8 pad02[0xA]; f32 x; u8 pad10[4]; f32 z;
    u8 pad18[0x4C]; Overlay1ObjectState *state;
} Overlay1Object;
typedef struct Overlay1ValueEntry { f32 value; u8 pad04[8]; } Overlay1ValueEntry;
typedef struct Overlay1ValueRow { Overlay1ValueEntry entries[6]; } Overlay1ValueRow;

extern Overlay1Object **overlay1GetObjectList(s32 *count);
extern s32 overlay1IsObjectActive(Overlay1Object *object);
extern f32 sqrtf(f32 value);
extern f32 overlay1TrigX(s32 angle);
extern f32 overlay1TrigY(s32 angle);
extern f32 D_160;
extern Overlay1ValueRow D_1BA8[];
extern s8 *D_1DA0;

#ifdef NON_MATCHING
Overlay1Object *overlay1FindDirectionalObject(Overlay1Object *object,
                                              void *unused1, void *unused2,
                                              f32 threshold, f32 maxValue) {
    Overlay1Object **objects;
    Overlay1Object *other;
    Overlay1Object *best;
    Overlay1ObjectState *otherState;
    s32 count;
    s32 remaining;
    f32 dx, dz, distance, directionX, directionY, dot, value, bestValue;

    objects = overlay1GetObjectList(&count);
    best = 0;
    bestValue = D_160;
    if (overlay1IsObjectActive(object)) {
        goto active;
    }
    return 0;
active:
    while (remaining = count--) {
        other = objects[count];
        if (other == object) {
        } else {
            dx = other->x - object->x;
            otherState = other->state;
            dz = other->z - object->z;
            distance = sqrtf((dx * dx) + (dz * dz));
            if (distance > 0.0f) {
                dx /= distance;
                dz /= distance;
            }
            directionX = -overlay1TrigX(object->angle);
            directionY = -overlay1TrigY(object->angle);
            dot = (directionX * dx) + (directionY * dz);
            if (DOT_CONDITION) {
                value = D_1BA8[*D_1DA0].entries[otherState->tableIndex].value;
                if ((value <= maxValue) && (value < bestValue)) {
                    bestValue = value;
                    best = other;
                }
            }
        }
    }
    return best;
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay1FindDirectionalObject/func_overlay_001_F0005CD4_18520B4.s")
#endif
