#include "ultra64.h"

typedef struct {
    s16 x;
    s16 y;
    s16 z;
    u8 r;
    u8 g;
    u8 b;
    u8 a;
} Overlay28Entry;

typedef struct {
    u8 pad0[0x42];
    s16 bufferIndex;
    Overlay28Entry entries[2][17];
} Overlay28State;

/* DKR v77/v80 have only generic double-buffered particle reset relatives. */
void overlay28ResetBuffer(Overlay28State *state, s32 count) {
    s32 color;
    Overlay28Entry *entry;

    count = 0x10;
    state->bufferIndex ^= 1;
    entry = state->entries[state->bufferIndex];
    color = 0xFF;
    do {
        entry->x = 0;
        entry->y = 0;
        entry->z = 0;
        entry->r = color;
        entry->g = color;
        entry->b = color;
        entry->a = color;
        entry++;
    } while (count--);
}
