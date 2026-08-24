#include "PR/ultratypes.h"

typedef struct Overlay84State {
    u8 pad0[9];
    s8 active;
} Overlay84State;

typedef struct Overlay84Object {
    u8 pad0[0x64];
    Overlay84State *state;
} Overlay84Object;

extern Overlay84Object *gOverlay84Object;

void overlay84ClearActive(void) {
    if (gOverlay84Object != NULL) {
        Overlay84State *state = gOverlay84Object->state;
        state->active = 0;
    }
}
