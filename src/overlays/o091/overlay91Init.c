#include "PR/ultratypes.h"

typedef struct Overlay91State {
    s32 timer;
    s32 mode;
} Overlay91State;

typedef struct Overlay91Object {
    s16 value0;
    u8 pad2[4];
    s16 flags6;
    u8 pad8[4];
    f32 minValue;
    f32 currentValue;
    f32 maxValue;
    u8 pad18[0x16];
    s16 index2E;
    u8 pad30[0x34];
    Overlay91State *state;
} Overlay91Object;

/* DKR v77/v80 and JFG checks found no exact donor for this object initializer. */
void overlay91Init(Overlay91Object *object, f32 unused) {
    Overlay91State *state = object->state;

    state->timer = 0;
    state->mode = 0;
    object->value0 = 0;
    object->flags6 |= 0x400;
    object->index2E = -1;
    object->minValue = -225.0f;
    object->currentValue = 0.0f;
    object->maxValue = 144.0f;
}
