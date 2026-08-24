#include "PR/ultratypes.h"

typedef struct Overlay84State {
    u8 pad0[0x3C];
    s32 first;
    s32 second;
} Overlay84State;

typedef struct Overlay84Object {
    u8 pad0[0x64];
    Overlay84State *state;
} Overlay84Object;

extern Overlay84Object *gOverlay84Object;

void overlay84GetValues(s32 *first, s32 *second) {
    if (gOverlay84Object != NULL) {
        Overlay84State *state = gOverlay84Object->state;
        *first = state->first;
        *second = state->second;
    }
}
