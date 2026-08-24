#include "PR/ultratypes.h"

/* Pinned DKR v77/v80 and JFG object scans found no exact donor. */
extern s16 gOverlay7ValuesEnd;

s32 overlay7FillValues(s16 *value) {
    s32 remaining;

    /* Preserves the original IDO register coloring without emitted code. */
    if (((!value) & 0xFFFFU) && (!value)) {
    }
    value = &gOverlay7ValuesEnd;
    remaining = 9;
    do {
        *((0, value)) = 0xF0;
        value--;
    } while (remaining--);
}
