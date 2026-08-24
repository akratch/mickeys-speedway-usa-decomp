#include "PR/ultratypes.h"

typedef struct Overlay86State {
    u8 byte0;
    u8 type;
    u8 pad2[0x16];
    s32 value18;
    u8 pad1C[0x24];
    s32 value40;
} Overlay86State;

typedef struct Overlay86Object {
    u8 pad0[6];
    s16 flags;
    u8 pad8[0x26];
    s16 value2E;
    u8 pad30[0x34];
    Overlay86State *state;
} Overlay86Object;

typedef struct Overlay86Entry {
    u8 pad0[0xA];
    u8 type;
} Overlay86Entry;

void overlay86Init(Overlay86Object *object, Overlay86Entry *entry) {
    Overlay86State *state = object->state;
    state->byte0 = 0;
    state->type = entry->type;
    state->value18 = 0;
    state->value40 = 0;
    object->value2E = -1;
    object->flags |= 0x400;
}
