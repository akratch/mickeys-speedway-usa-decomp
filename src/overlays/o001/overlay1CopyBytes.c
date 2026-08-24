#include "PR/ultratypes.h"

/* DKR v77/v80 contains only generic byte-copy initialization patterns. */
typedef struct Overlay1ByteState {
    u8 values[6];
} Overlay1ByteState;

typedef struct Overlay1ByteObject {
    u8 pad0[0x64];
    Overlay1ByteState *state;
} Overlay1ByteObject;

typedef struct Overlay1ByteInit {
    u8 pad0[0xA];
    u8 values[6];
} Overlay1ByteInit;

void overlay1CopyBytes(Overlay1ByteObject *object, Overlay1ByteInit *init) {
    Overlay1ByteState *state = object->state;

    state->values[0] = init->values[0];
    state->values[1] = init->values[1];
    state->values[2] = init->values[2];
    state->values[3] = init->values[3];
    state->values[4] = init->values[4];
    state->values[5] = init->values[5];
}
