#include "ultra64.h"

typedef struct {
    u8 pad0[0x38D];
    u8 state;
    s16 timer;
    u8 pad390[0x26];
    s16 valueA;
    s16 valueB;
} Overlay3State;

typedef struct {
    u8 pad0[0x64];
    Overlay3State *state;
} Overlay3Object;

extern Overlay3Object **overlay3GetObjectsReloc(s32 *count);

/* DKR v77/v80 only provide generic active-object reset relatives. */
void overlay3ResetObjects(void) {
    s32 count;
    Overlay3Object **objects;
    Overlay3State *state;

    objects = overlay3GetObjectsReloc(&count);
    if (count--) {
        do {
            state = objects[count]->state;
            state->state = 0x7F;
            state->timer = 0;
            state->valueA = 0;
            state->valueB = 0;
        } while (count--);
    }
}
