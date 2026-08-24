#include "PR/ultratypes.h"

typedef struct Overlay4Object Overlay4Object;

typedef struct Overlay4State {
    Overlay4Object *owner;
    u8 pad04[2];
    u8 flags;
    u8 pad07[4];
    u8 marker;
} Overlay4State;

typedef struct Overlay4OwnerState {
    s8 group;
} Overlay4OwnerState;

struct Overlay4Object {
    u8 pad00[0x64];
    void *state;
};

typedef struct Overlay4Group {
    Overlay4Object *objects[0x20];
    s32 count;
} Overlay4Group;

extern Overlay4Group gOverlay4Groups[];

/* DKR v77/v80 and JFG contain no exact donor for this group attachment. */
void overlay4AttachObject(Overlay4Object *owner, Overlay4Object *object) {
    Overlay4OwnerState *ownerState;
    Overlay4State *state;
    Overlay4Group *group;

    ownerState = owner->state;
    state = object->state;
    group = &gOverlay4Groups[ownerState->group];
    /* Keeping these on one source line reproduces IDO's original schedule. */
    group->objects[group->count] = object; group->count++;
    state->owner = owner;
    state->flags |= 0xC;
    state->marker = 0xFF;
}
