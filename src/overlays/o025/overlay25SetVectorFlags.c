#include "PR/ultratypes.h"

/* No corresponding DKR/JFG source or object match was found. */
typedef struct Overlay25Vector {
    f32 x, y, z;
} Overlay25Vector;

typedef struct Overlay25Source {
    u8 pad0[4];
    f32 value;
    u8 pad8[8];
    Overlay25Vector vector;
    u8 pad1C[4];
    s32 flags;
} Overlay25Source;

typedef struct Overlay25State {
    u8 pad0[0x14];
    s32 flags;
} Overlay25State;

typedef struct Overlay25Object {
    u8 pad0[0x64];
    Overlay25State *state;
} Overlay25Object;

extern f32 gOverlay25Threshold;

void overlay25SetVectorFlags(s32 unused0, Overlay25Vector *out, s32 unused2,
                             s32 unused3, Overlay25Source *source,
                             Overlay25Object *object) {
    Overlay25State *state;

    state = object->state;
    out->x = source->vector.x;
    out->y = source->vector.y;
    out->z = source->vector.z;
    if ((gOverlay25Threshold < source->value) || (source->flags & 0x10000000)) {
        state->flags |= 2;
        return;
    }
    state->flags |= 4;
}
