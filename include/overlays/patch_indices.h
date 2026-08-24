#ifndef OVERLAYS_PATCH_INDICES_H
#define OVERLAYS_PATCH_INDICES_H

#include "PR/ultratypes.h"

typedef struct OverlayPatchIndexEntry {
    s32 first;
    s32 second;
    u8 pad8[8];
} OverlayPatchIndexEntry;

#endif
