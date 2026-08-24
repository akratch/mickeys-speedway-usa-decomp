#include "PR/ultratypes.h"

/* Small state helpers; the exact DKR and JFG donor scans are negative. */
typedef struct Overlay81Object {
    u8 pad0[0x64];
    void *state;
} Overlay81Object;

typedef struct Overlay81Init {
    u8 pad0[0xA];
    u16 radius;
    u16 index;
} Overlay81Init;

typedef struct Overlay81State {
    f32 radius;
    s32 index;
} Overlay81State;

extern s32 gOverlay81Mask;

void overlay81SetMaskBit(s32 bit) {
    gOverlay81Mask |= 1 << bit;
}

void overlay81InitState(Overlay81Object *object, Overlay81Init *init) {
    Overlay81State *state = object->state;

    state->radius = init->radius;
    state->index = init->index;
}
