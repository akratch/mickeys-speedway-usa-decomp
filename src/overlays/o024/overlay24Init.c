#include "PR/ultratypes.h"

/* Small state initializer; exact DKR and JFG scans are negative. */
typedef struct Overlay24State {
    u8 mode;
    s8 remaining;
    u8 pad2[0xE];
    void *target;
} Overlay24State;

typedef struct Overlay24Object {
    u8 pad0[0x64];
    Overlay24State *state;
} Overlay24Object;

typedef struct Overlay24InitData {
    u8 pad0[0xA];
    s16 remaining;
    void *target;
} Overlay24InitData;

void overlay24Init(Overlay24Object *object, Overlay24InitData *init) {
    Overlay24State *state;

    state = object->state;
    state->mode = 0;
    state->remaining = init->remaining;
    state->target = init->target;
}
