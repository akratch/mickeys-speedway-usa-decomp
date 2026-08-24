#include "PR/ultratypes.h"

typedef struct Overlay84CurrentState {
    u8 pad0;
    s8 current;
    s8 status;
} Overlay84CurrentState;
typedef struct Overlay84CurrentObject {
    u8 pad0[0x64];
    Overlay84CurrentState *state;
} Overlay84CurrentObject;
extern Overlay84CurrentObject *gOverlay84Object;

s32 overlay84GetCurrent(void) {
    Overlay84CurrentState *state;

    if (gOverlay84Object == 0) {
        return 0;
    }
    state = gOverlay84Object->state;
    if (state->status == -1) {
        return 0;
    }
    return state->current;
}
