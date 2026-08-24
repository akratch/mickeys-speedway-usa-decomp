#include "PR/ultratypes.h"

/* Exact DKR and JFG scans are negative. */
typedef struct Overlay27State {
    u8 byte0, byte1; s16 half2; u8 r, g, b, r2, g2, b2;
    s16 halfA, halfC, halfE; f32 value10, value14; s32 word18, word1C; void *target;
} Overlay27State;
typedef struct Overlay27Object { u8 pad0[0x64]; Overlay27State *state; } Overlay27Object;
typedef struct Overlay27InitData { u8 pad0[0xC]; void *target; } Overlay27InitData;

void overlay27Init(Overlay27Object *object, Overlay27InitData *init) {
    Overlay27State *state = object->state;
    state->byte0 = 0; state->byte1 = 0; state->half2 = 0;
    state->r = 0xFF; state->g = 0xFF; state->b = 0xFF;
    state->r2 = 0x40; state->g2 = 0x40; state->b2 = 0x40;
    state->halfA = 0; state->halfC = 0; state->halfE = 0;
    state->word18 = 0; state->word1C = 0;
    state->value10 = 96.0f; state->value14 = 0.0f; state->target = init->target;
}
