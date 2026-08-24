#include "PR/ultratypes.h"

typedef struct Overlay81Transform {
    f32 scale;
    u8 pad4[0x50];
    f32 width;
    f32 depth;
} Overlay81Transform;

typedef struct Overlay81Collision {
    s16 flags;
    u8 pad2[2];
    f32 radius;
} Overlay81Collision;

typedef struct Overlay81State {
    f32 x;
    f32 y;
    f32 z;
    s32 timer;
    u8 pad10[4];
    s32 mask;
} Overlay81State;

typedef struct Overlay81Object {
    u8 pad0[8];
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x28];
    Overlay81Transform *transform;
    u8 pad44[8];
    f32 *dimensions;
    u8 pad50[0x14];
    Overlay81State *state;
    u8 pad68[0x10];
    Overlay81Collision *collision;
} Overlay81Object;

typedef struct Overlay81Init {
    u8 pad0[0xA];
    s16 scale;
    s16 timer;
    s16 bit;
} Overlay81Init;

extern f32 gOverlay81Scale;

void overlay81Init(Overlay81Object *object, Overlay81Init *init, s32 unused) {
    Overlay81State *state;
    Overlay81Collision *collision;
    s32 bit;

    object->scale = (f32) init->scale * gOverlay81Scale;
    state = object->state;
    if (object->dimensions != 0) {
        object->dimensions[0] = object->transform->width * object->scale;
        object->dimensions[1] = object->transform->depth * object->scale;
    }
    object->scale *= object->transform->scale;
    collision = object->collision;
    collision->radius *= object->scale;
    state->x = object->x;
    state->y = object->y;
    state->z = object->z;
    state->timer = init->timer;
    bit = init->bit;
    if (bit != -1) {
        state->mask = 1 << bit;
    }
    collision = object->collision;
    collision->flags |= 2;
}
