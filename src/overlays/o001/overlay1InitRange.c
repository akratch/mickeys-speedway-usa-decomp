#include "PR/ultratypes.h"

/* Compact range initializer; exact DKR and JFG scans are negative. */
typedef struct Overlay1RangeState { s16 a, b; s32 value; u8 c, d; } Overlay1RangeState;
typedef struct Overlay1Object { u8 pad0[0x64]; Overlay1RangeState *state; } Overlay1Object;
typedef struct Overlay1RangeInit { u8 pad0[0xA]; u8 a, b, value, c, d; } Overlay1RangeInit;
void overlay1InitRange(Overlay1Object *object, Overlay1RangeInit *init) {
    Overlay1RangeState *state = object->state;
    state->a = init->a * 10; state->b = init->b * 10;
    state->value = init->value; state->c = init->c; state->d = init->d;
}
