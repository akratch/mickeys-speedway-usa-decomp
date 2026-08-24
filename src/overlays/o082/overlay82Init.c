#ifndef PERMUTER
#include "PR/ultratypes.h"
#else
typedef signed char s8;
typedef unsigned char u8;
typedef signed int s32;
typedef unsigned int u32;
typedef float f32;
#endif

typedef struct Overlay82State {
    s8 selection;
    s8 changed;
    u8 active;
    u8 disabled;
    s32 values[6];
} Overlay82State;

typedef struct Overlay82Object {
    u8 pad0[0x64];
    void *state;
} Overlay82Object;

/* DKR v77/v80 and JFG contain no exact donor for this state initializer. */
void overlay82Init(Overlay82Object *object, f32 updateRate) {
    u8 *state;
    s32 *values;
    s32 index;

    state = object->state;
    values = (s32 *) state;
    state[0] = 0;
    state[1] = 0;
    state[2] = 0;
    state[3] = 0;
    for (index = 0; index < 2; index++) values[index + 1] = 0;
    object = (Overlay82Object *)((u8 *)values + index * sizeof(s32));
    ((s32 *)object)[2] = 0;
    ((s32 *)object)[3] = 0;
    ((s32 *)object)[4] = 0;
    ((s32 *)object)[1] = 0;
}
