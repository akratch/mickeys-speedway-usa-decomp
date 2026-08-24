#include "PR/ultratypes.h"

typedef struct Overlay84ScaleState { u8 pad0[0x2C]; f32 scale; } Overlay84ScaleState;
typedef struct Overlay84ScaleObject { u8 pad0[0x64]; Overlay84ScaleState *state; } Overlay84ScaleObject;
extern Overlay84ScaleObject *gOverlay84Object;

s32 overlay84IsUnitScale(void) {
    Overlay84ScaleState *state;

    if (gOverlay84Object == 0) {
        return 0;
    }
    state = gOverlay84Object->state;
    return state->scale == 1.0f;
}
