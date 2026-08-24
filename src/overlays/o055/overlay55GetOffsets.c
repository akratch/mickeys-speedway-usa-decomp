#include "ultra64.h"

extern s16 gOverlay55Offsets[];
extern s32 overlay55QueryModeReloc(void);

/* Shared Mickey family; no exact DKR v77/v80 or JFG donor was found. */
void overlay55GetOffsets(s32 mode, s32 index, s32 *xOffset, s32 *yOffset) {
    s16 *offsets;

    index *= 4;
    if (overlay55QueryModeReloc() == 1) {
        index += 4;
    }

    offsets = &gOverlay55Offsets[index];
    if (mode < 2) {
        *yOffset = offsets[1];
    } else {
        *yOffset = offsets[3];
    }
    if (mode & 1) {
        *xOffset = offsets[2];
    } else {
        *xOffset = offsets[0];
    }
}
