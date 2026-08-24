#include "PR/ultratypes.h"

typedef struct Overlay1OffsetState {
    u8 pad00[0x383];
    s8 cycle;
    u8 pad384[0x18];
    f32 offset;
} Overlay1OffsetState;

typedef struct Overlay1OffsetObject {
    u8 pad00[0x64];
    Overlay1OffsetState *state;
} Overlay1OffsetObject;

/* Pinned DKR v77/v80 and JFG object scans found no exact donor. */
extern s32 gOverlay1CycleLength;

f32 overlay1SignedOffset(Overlay1OffsetObject *first,
                         Overlay1OffsetObject *second) {
    Overlay1OffsetState *firstState;
    Overlay1OffsetState *secondState;
    f32 firstOffset;
    f32 secondOffset;

    firstState = first->state;
    secondState = second->state;
    firstOffset = (firstState->cycle * gOverlay1CycleLength) +
                  firstState->offset;
    secondOffset = (secondState->cycle * gOverlay1CycleLength) +
                   secondState->offset;
    return firstOffset - secondOffset;
}
