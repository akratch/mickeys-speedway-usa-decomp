#include "PR/ultratypes.h"

typedef struct Overlay79Object {
    u8 pad0[0x64];
    s32 *state;
} Overlay79Object;

typedef struct Overlay79Entry {
    u8 pad0[0xC];
    s32 value;
} Overlay79Entry;

void overlay79SetLink(Overlay79Object *object, Overlay79Entry *entry) {
    s32 *state = object->state;
    state[1] = entry->value;
}
