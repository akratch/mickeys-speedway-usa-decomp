#include "PR/ultratypes.h"

typedef struct Overlay84State {
    u8 pad0[0xE];
    s16 angle;
} Overlay84State;

typedef struct Overlay84Object {
    u8 pad0[0x64];
    Overlay84State *state;
} Overlay84Object;

extern Overlay84Object *gOverlay84Object;

void overlay84SetAngle(s16 angle) {
    if (gOverlay84Object != NULL) {
        Overlay84State *state = gOverlay84Object->state;
        state->angle = angle;
    }
}
