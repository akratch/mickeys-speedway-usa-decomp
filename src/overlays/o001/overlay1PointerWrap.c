#include "PR/ultratypes.h"

/* Pinned DKR/JFG scans have no exact donor; these are generic ring helpers. */
extern u8 *gOverlay1Start;
extern u8 *gOverlay1End;

u8 *overlay1PreviousPointer(u8 *pointer) {
    pointer -= 0x94;
    if (pointer < gOverlay1Start) {
        pointer = gOverlay1End;
    }
    return pointer;
}

u8 *overlay1NextPointer(u8 *pointer) {
    pointer += 0x94;
    if (pointer > gOverlay1End) {
        pointer = gOverlay1Start;
    }
    return pointer;
}
