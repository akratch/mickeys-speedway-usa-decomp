#include "PR/ultratypes.h"

typedef struct Overlay79Object {
    s16 value;
    u8 pad2[0x62];
    s32 *state;
} Overlay79Object;

typedef struct Overlay79Entry {
    u8 pad0[0xA];
    s16 value;
} Overlay79Entry;

void overlay79InitState(Overlay79Object *object, Overlay79Entry *entry) {
    s32 *state = object->state;
    state[1] = 0xFF;
    state[0] = 0x78;
    object->value = entry->value;
}
