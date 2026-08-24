#include "PR/ultratypes.h"

typedef struct Overlay84State {
    u8 pad0[0xC6];
    u8 marked;
} Overlay84State;

typedef struct Overlay84Object {
    u8 pad0[0x64];
    Overlay84State *state;
} Overlay84Object;

extern Overlay84Object *gOverlay84Object;

void overlay84Mark(void) {
    if (gOverlay84Object != NULL) {
        Overlay84State *state = gOverlay84Object->state;
        state->marked = 1;
    }
}
