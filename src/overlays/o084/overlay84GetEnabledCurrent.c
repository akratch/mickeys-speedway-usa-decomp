#include "PR/ultratypes.h"

typedef struct Overlay84EnabledState {
    u8 pad0;
    s8 current;
    s8 status;
    u8 pad3[3];
    s8 enabled;
} Overlay84EnabledState;
typedef struct Overlay84EnabledObject { u8 pad0[0x64]; Overlay84EnabledState *state; } Overlay84EnabledObject;
extern Overlay84EnabledObject *gOverlay84Object;

s32 overlay84GetEnabledCurrent(void) {
    Overlay84EnabledState *state;

    if (gOverlay84Object == 0) {
        return -1;
    }
    state = gOverlay84Object->state;
    if (state->status == -1) {
        return -1;
    }
    if (state->enabled != 0) {
        return state->current;
    }
    return -1;
}
