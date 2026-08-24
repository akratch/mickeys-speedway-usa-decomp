#include "PR/ultratypes.h"

typedef struct Overlay1MaskedState {
    u8 pad0[0x382];
    u8 bitIndex;
    u8 pad383[0xD];
    u8 timer;
} Overlay1MaskedState;

typedef struct Overlay1MaskedObject {
    u8 pad0[0x64];
    Overlay1MaskedState *state;
} Overlay1MaskedObject;

extern u8 gOverlay1ModeMasks[];
extern void overlay1SelectModeReloc(void *, s32);

/* DKR v77/v80 and JFG contain no exact donor for this table-mask selector. */
s32 overlay1SelectMaskedMode(Overlay1MaskedObject *object, s32 index) {
    Overlay1MaskedState *state;

    state = object->state;
    if (*(u16 *)(gOverlay1ModeMasks + index * 12 + 0xC4) &
        (1 << state->bitIndex)) {
        state->bitIndex = index;
        overlay1SelectModeReloc(&state->timer, 8);
        return 1;
    }
    return 0;
}
