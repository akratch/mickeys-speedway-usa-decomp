#include "PR/ultratypes.h"

/* No corresponding DKR/JFG source or object match was found. */
typedef struct Overlay1MotionState {
    f32 magnitude;
    s32 mode;
    u16 first;
    u16 second;
} Overlay1MotionState;

typedef struct Overlay1MotionObject {
    u8 pad0[8];
    f32 scaledMagnitude;
    u8 padC[0x58];
    Overlay1MotionState *state;
} Overlay1MotionObject;

typedef struct Overlay1MotionInit {
    u8 pad0[0xA];
    u8 magnitude;
    u8 mode;
    u16 first;
    u16 second;
} Overlay1MotionInit;

extern f32 gOverlay1MotionScale;

void overlay1InitMotion(Overlay1MotionObject *object, Overlay1MotionInit *init) {
    Overlay1MotionState *state;

    state = object->state;
    object->scaledMagnitude = (f32)init->magnitude * gOverlay1MotionScale;
    state->magnitude = (f32)init->magnitude;
    state->mode = init->mode;
    state->first = init->first;
    state->second = init->second;
}
