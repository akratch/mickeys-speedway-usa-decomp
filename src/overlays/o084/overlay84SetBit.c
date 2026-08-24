#include "PR/ultratypes.h"

typedef struct Overlay84State {
    u8 pad0[0xC8];
    u32 flags;
} Overlay84State;

typedef struct Overlay84Object {
    u8 pad0[0x64];
    Overlay84State *state;
} Overlay84Object;

extern Overlay84Object *gOverlay84Object;

void overlay84SetBit(s32 index) {
    if (gOverlay84Object != NULL) {
        gOverlay84Object->state->flags |= 1 << index;
    }
}
