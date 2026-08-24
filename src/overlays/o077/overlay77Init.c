#include "PR/ultratypes.h"

/* DKR object initializers supplied the radius/clamp/divide source idiom. */

typedef struct Overlay77State {
    s16 kind;
    s16 sequence;
    f32 acceleration;
    f32 scale;
    f32 targetY;
    f32 targetX;
    f32 targetYCopy;
    f32 targetZ;
} Overlay77State;

typedef struct Overlay77Header {
    f32 scale;
} Overlay77Header;

typedef struct Overlay77Object {
    s16 angle;
    u8 pad2[6];
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[4];
    f32 velocityX;
    u8 pad20[4];
    f32 velocityZ;
    f32 field28;
    u8 pad2C[0x14];
    Overlay77Header *header;
    u8 pad44[0x20];
    Overlay77State *state;
} Overlay77Object;

typedef struct Overlay77Init {
    u8 pad0[0xA];
    u8 kind;
    u8 radius;
    s16 fieldC;
    s16 angle;
} Overlay77Init;

extern s32 gOverlay77Sequence;

f32 overlay77SinReloc(s16 angle);
f32 overlay77CosReloc(s16 angle);

void overlay77Init(Overlay77Object *object, Overlay77Init *init, s32 preserveSequence) {
    Overlay77State *state;
    f32 radius;

    radius = init->radius & 0xFF;
    state = object->state;
    if (radius < 10.0f) {
        radius = 10.0f;
    }
    radius = radius / 64.0f;
    object->scale = object->header->scale * radius;
    object->field28 = (f32) init->fieldC;
    state->kind = init->kind;
    state->scale = 5.0f;
    state->targetY = object->y + 100.0f;
    state->targetX = object->x;
    state->targetYCopy = object->y;
    state->targetZ = object->z;
    object->angle = init->angle;
    object->velocityX = overlay77SinReloc(object->angle) * -20.0f;
    object->velocityZ = overlay77CosReloc(object->angle) * -20.0f;
    if (preserveSequence == 0) {
        state->sequence = gOverlay77Sequence;
        gOverlay77Sequence++;
    }
}
