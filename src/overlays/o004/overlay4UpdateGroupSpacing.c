#include "PR/ultratypes.h"

typedef struct Overlay4ObjectState {
    s8 group;
} Overlay4ObjectState;

typedef struct Overlay4ChainObject {
    u8 pad00[0xC];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x4C];
    Overlay4ObjectState *state;
} Overlay4ChainObject;

typedef struct Overlay4Group {
    Overlay4ChainObject *objects[32];
    s32 count;
} Overlay4Group;

extern Overlay4Group gOverlay4Groups[];
extern f32 sqrtf(f32 value);

void overlay4UpdateGroupSpacing(Overlay4ChainObject *object) {
    Overlay4Group *group;
    Overlay4ChainObject **objects;
    Overlay4ChainObject *previous;
    s32 count;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 distance;
    f32 scale;

    group = &gOverlay4Groups[object->state->group];
    count = group->count;
    objects = group->objects;
    while (count--) {
        previous = object;
        object = *objects++;
        dx = object->x - previous->x;
        dy = object->y - previous->y;
        dz = object->z - previous->z;
        distance = sqrtf((dx * dx) + (dy * dy) + (dz * dz));
        scale = distance;
        if (distance > 0.0f) {
            scale = 38.0f / distance;
        }
        dx *= scale;
        dy *= scale;
        dz *= scale;
        object->x = previous->x + dx;
        object->y = previous->y + dy;
        object->z = previous->z + dz;
    }
}
