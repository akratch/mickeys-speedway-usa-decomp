#include "PR/ultratypes.h"

/* DKR v77/v80 semantic-source search: negative (generic field access only). */
typedef struct Overlay82State {
    s8 selection;
    s8 changed;
    u8 active;
    u8 disabled;
} Overlay82State;

typedef struct Overlay82Object {
    u8 pad0[0x64];
    Overlay82State *state;
} Overlay82Object;

s32 overlay82GetSelection(Overlay82Object *object) {
    Overlay82State *state;

    state = object->state;
    return state->selection;
}

u32 overlay82IsActive(Overlay82Object *object) {
    Overlay82State *state;

    state = object->state;
    return state->active;
}

void overlay82Disable(Overlay82Object *object) {
    Overlay82State *state;

    state = object->state;
    state->disabled = 1;
}

void overlay82Enable(Overlay82Object *object) {
    Overlay82State *state;

    state = object->state;
    state->disabled = 0;
}
