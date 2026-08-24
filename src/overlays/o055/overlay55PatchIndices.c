#include "overlays/patch_indices.h"

extern void *gOverlay55Objects[];

/* This exact six-overlay Mickey family has no exact DKR v77/v80 or JFG donor. */
void overlay55PatchIndices(OverlayPatchIndexEntry *entry) {
    while (entry->first != 0) {
        entry->first = (s32) gOverlay55Objects[entry->first];
        if (entry->second != 0) {
            entry->second = (s32) gOverlay55Objects[entry->second];
        }
        entry++;
    }
}
