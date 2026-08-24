#include "PR/ultratypes.h"

typedef struct Overlay84ActiveState {
    u8 pad0[9];
    u8 active;
} Overlay84ActiveState;
typedef struct Overlay84ActiveObject {
    u8 pad0[0x64];
    Overlay84ActiveState *state;
} Overlay84ActiveObject;

extern Overlay84ActiveObject *gOverlay84Object;

s32 overlay84GetActive(void) {
    Overlay84ActiveState *state;

    if (gOverlay84Object == 0) {
        return -1;
    }
    state = gOverlay84Object->state;
    return state->active;
}
