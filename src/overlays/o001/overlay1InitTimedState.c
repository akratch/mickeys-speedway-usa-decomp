#include "PR/ultratypes.h"

/* DKR v77/v80 contains only generic object-state initialization patterns. */
typedef struct Overlay1TimedState {
    u8 pad0[0x193];
    u8 field193;
    u8 pad194[0x1EE];
    u8 enabled;
    u8 pad383[0xD];
    s16 timer;
    u8 pad392[0x52];
    s32 value3E4;
} Overlay1TimedState;

typedef struct Overlay1TimedObject {
    u8 pad0[0x64];
    Overlay1TimedState *state;
} Overlay1TimedObject;

void overlay1InitTimedState(Overlay1TimedObject *object, s32 timer) {
    Overlay1TimedState *state = object->state;

    state->enabled = 1;
    state->timer = timer;
    state->field193 = 0;
    state->value3E4 = 0;
}
