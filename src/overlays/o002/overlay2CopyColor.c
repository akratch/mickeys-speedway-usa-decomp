#include "PR/ultratypes.h"

typedef struct Overlay2ColorObject {
    u8 pad0[0x64];
    u8 *state;
} Overlay2ColorObject;

/* DKR v77/v80 and JFG checks found only generic three-byte color copies. */
void overlay2CopyColor(Overlay2ColorObject *object, u8 *color) {
    u8 *state = object->state;

    state[0] = color[0xA];
    state[1] = color[0xB];
    state[2] = color[0xC];
}
