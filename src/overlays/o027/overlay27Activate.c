#include "PR/ultratypes.h"

typedef struct Overlay27State {
    u8 mode;
    u8 pad1;
    s16 value;
} Overlay27State;

typedef struct Overlay27Object {
    u8 pad00[0x64];
    Overlay27State *state;
    u8 pad68[0x29];
    u8 blocked;
} Overlay27Object;

/* DKR v77/v80 and JFG contain no exact donor for this state transition. */
s32 overlay27Activate(Overlay27Object *object) {
    Overlay27Object *savedObject;

    if (object != 0 && object->blocked == 0) {
        if (object->state->mode == 4) {
            (savedObject = object)->state->mode = 3;
            if (object->state == 0 && object->state == 0) {
            }
            return 1;
        }
        if (object->state->mode == 2) {
            object->state->value = 0;
        }
        return 1;
    }
    return 0;
}
