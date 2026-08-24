#include "PR/ultratypes.h"

typedef struct Overlay1LinkHeader {
    u8 pad0[7];
    u8 active;
} Overlay1LinkHeader;

typedef struct Overlay1LinkObject {
    u8 pad0[0x64];
    Overlay1LinkHeader *header;
} Overlay1LinkObject;

typedef struct Overlay1LinkState {
    u8 pad0[0x382];
    u8 mode;
    u8 pad383[0x11];
    Overlay1LinkObject *linked;
} Overlay1LinkState;

typedef struct Overlay1LinkOwner {
    u8 pad0[0x64];
    Overlay1LinkState *state;
} Overlay1LinkOwner;

/* Pinned DKR v77/v80 and JFG scans classify overlay 1 as no donor. */
Overlay1LinkObject *overlay1GetLinkedActive(Overlay1LinkOwner *owner) {
    Overlay1LinkState *state;
    Overlay1LinkObject *linked;

    if (owner != NULL && (state = owner->state) != NULL &&
        ((1 << state->mode) & 0x1C) != 0 &&
        (linked = state->linked) != NULL && linked->header->active != 0) {
        return linked;
    }
    return NULL;
}
