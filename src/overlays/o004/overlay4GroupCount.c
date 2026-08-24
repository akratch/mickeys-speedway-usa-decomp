#include "PR/ultratypes.h"

typedef struct Overlay4ObjectState {
    s8 group;
} Overlay4ObjectState;

typedef struct Overlay4Object {
    u8 pad00[0x64];
    Overlay4ObjectState *state;
} Overlay4Object;

typedef struct Overlay4Group {
    u8 pad00[0x80];
    s32 count;
} Overlay4Group;

/* Pinned DKR v77/v80 and JFG object scans found no exact donor. */
extern Overlay4Group gOverlay4Groups[];

s32 overlay4GroupCount(Overlay4Object *object) {
    Overlay4ObjectState *state;

    state = object->state;
    return gOverlay4Groups[state->group].count;
}
