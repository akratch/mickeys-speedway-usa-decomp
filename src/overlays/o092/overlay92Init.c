#include "PR/ultratypes.h"

/* Exact DKR and JFG scans are negative. */
typedef struct Overlay92State { s16 x, y, z, w; u16 a, b; s8 mode; } Overlay92State;
typedef struct Overlay92Object { s16 x, y, z; u8 pad6[0x5E]; Overlay92State *state; } Overlay92Object;
typedef struct Overlay92InitData {
    u8 pad0[0xA]; s8 x, y, z, mode; u16 a, b; u8 w;
} Overlay92InitData;

void overlay92Init(Overlay92Object *object, Overlay92InitData *init) {
    Overlay92State *state = object->state;
    state->x = init->x << 8; state->y = init->y << 8; state->z = init->z << 8;
    state->w = init->w << 8; state->a = init->a; state->b = init->b; state->mode = init->mode;
    object->x = state->z; object->y = state->y; object->z = state->x;
}
