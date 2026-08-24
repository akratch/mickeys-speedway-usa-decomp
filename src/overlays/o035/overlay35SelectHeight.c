#include "ultra64.h"

typedef struct {
    u8 pad0[2];
    s16 value;
    u8 pad4[6];
} Overlay35Value;

typedef struct {
    u8 pad0[6];
    s16 valueIndex;
    u8 pad8[4];
    u32 flags;
} Overlay35Record;

typedef struct {
    Overlay35Value *values;
    u8 pad4[8];
    Overlay35Record *records;
    u8 pad10[0x14];
    s16 count;
    u8 pad26[0x16];
    s16 selectedValue;
} Overlay35State;

/* DKR v77/v80 and JFG have no exact flagged-record height selector. */
void overlay35SelectHeight(Overlay35State *state) {
    s32 index;
    Overlay35Record *record;

    index = 0;
    state->selectedValue = -10000;
    if (state->count > 0) {
        record = state->records;
        do {
            index++;
            if (record->flags & 0x10000) {
                state->selectedValue = state->values[record->valueIndex].value;
            }
            record++;
        } while (index < state->count);
    }
}
