#include "PR/ultratypes.h"

typedef struct Overlay68NestedState { u8 pad0[0xE]; u8 flag; } Overlay68NestedState;
typedef struct Overlay68NestedObject { u8 pad0[0x64]; Overlay68NestedState *state; } Overlay68NestedObject;

void overlay68ClearNestedFlag(Overlay68NestedObject **objectPtr) {
    Overlay68NestedObject *object;
    Overlay68NestedState *state;

    if (objectPtr != 0) {
        object = *objectPtr;
        if (object != 0) {
            state = object->state;
            if (state != 0) {
                state->flag = 0;
            }
        }
    }
}
