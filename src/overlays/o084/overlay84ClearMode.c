#include "PR/ultratypes.h"

typedef struct Overlay84State {
    u8 pad0[6];
    s8 mode;
} Overlay84State;

typedef struct Overlay84Object {
    u8 pad0[0x64];
    Overlay84State *state;
} Overlay84Object;

extern Overlay84Object *gOverlay84Object;

void overlay84ClearMode(void) {
    if (gOverlay84Object != NULL) {
        Overlay84State *state = gOverlay84Object->state;
        state->mode = 0;
    }
}
