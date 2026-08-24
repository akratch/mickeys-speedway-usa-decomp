#include "PR/ultratypes.h"

/* Both local DKR v77/v80 object and semantic-signature scans are negative. */
typedef struct Overlay37Target {
    u8 pad00[0xC];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x16];
    s16 angle;
} Overlay37Target;

typedef struct Overlay37State {
    f32 x;
    f32 y;
    f32 z;
    u8 pad0C[4];
    Overlay37Target *target;
} Overlay37State;

typedef struct Overlay37Object {
    u8 pad00[0xC];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x16];
    s16 angle;
    u8 pad30[0x34];
    Overlay37State *state;
} Overlay37Object;

typedef struct Overlay37Init {
    u8 pad00[4];
    s16 x;
    s16 y;
    s16 z;
    u8 pad0A[2];
    Overlay37Target *target;
} Overlay37Init;

void overlay37Init(Overlay37Object *object, Overlay37Init *init) {
    Overlay37State *state;

    state = object->state;
    state->x = init->x;
    state->y = init->y;
    state->z = init->z;
    state->target = init->target;
    if (init->target != 0) {
        object->x = init->target->x;
        object->y = init->target->y;
        object->z = init->target->z;
        object->angle = init->target->angle;
    }
}
