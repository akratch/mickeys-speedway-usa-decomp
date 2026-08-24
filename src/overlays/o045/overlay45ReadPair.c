#include "PR/ultratypes.h"

typedef struct Overlay45PairRecord {
    f32 x;
    f32 y;
    u8 pad8[0x1C];
} Overlay45PairRecord;

typedef struct Overlay45PairOwner {
    Overlay45PairRecord *records;
} Overlay45PairOwner;

/* DKR v77/v80 and JFG checks found no exact donor for this coordinate accessor. */
void overlay45ReadPair(Overlay45PairOwner *owner, s32 *x, s32 *y, s32 index) {
    Overlay45PairRecord *record = &owner->records[index];

    *x = record->x;
    *y = record->y;
}
