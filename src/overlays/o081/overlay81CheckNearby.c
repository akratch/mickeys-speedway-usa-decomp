#include "PR/ultratypes.h"

/* Radius query; exact DKR and JFG scans are negative. */
typedef struct Overlay81State {
    f32 radius;
    s32 index;
} Overlay81State;

typedef struct Overlay81Object {
    u8 pad0[0xC];
    f32 x;
    u8 pad10[4];
    f32 z;
    u8 pad18[0x4C];
    Overlay81State *state;
} Overlay81Object;

typedef struct Overlay81NearbyObject {
    u8 pad0[0xC];
    f32 x;
    u8 pad10[4];
    f32 z;
} Overlay81NearbyObject;

extern s32 gOverlay81Mask;

Overlay81NearbyObject **overlay81QueryNearbyReloc(s32 *count,
                                                   Overlay81Object *object,
                                                   Overlay81State *state);

void overlay81CheckNearby(Overlay81Object *object, s32 unused) {
    Overlay81State *state;
    Overlay81NearbyObject **nearby;
    Overlay81NearbyObject *candidate;
    f32 radiusSquared;
    f32 deltaX;
    f32 deltaZ;
    u8 found;
    s32 count;
    volatile u64 scratch;

    state = object->state;
    radiusSquared = state->radius;
    radiusSquared *= radiusSquared;
    nearby = overlay81QueryNearbyReloc(&count, object, state);
    found = 0;
    if (count != 0) {
        do {
            candidate = *nearby++;
            deltaX = candidate->x - object->x;
            deltaZ = candidate->z - object->z;
            if (((deltaX * deltaX) + (deltaZ * deltaZ)) < radiusSquared) {
                found = 1;
            }
            count = count - 1;
        } while (count != 0);
    }
    if (found != 0) {
        gOverlay81Mask |= 1 << state->index;
    }
}
