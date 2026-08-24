#include "PR/ultratypes.h"

typedef struct Overlay2Collection {
    u8 pad0[4];
    s32 count;
} Overlay2Collection;

/* DKR v77/v80 and JFG have no exact donor for this circular-index helper. */
void overlay2AdjacentIndices(Overlay2Collection *collection, u16 index,
                             s16 *previous, s16 *next) {
    *previous = index - 1;
    *next = index + 1;
    if (index == 0) {
        *previous = collection->count - 1;
        return;
    }
    if (index == collection->count - 1) {
        *next = 0;
    }
}
