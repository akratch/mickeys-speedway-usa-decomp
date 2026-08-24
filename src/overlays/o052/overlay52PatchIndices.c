#include "overlays/patch_indices.h"

extern void *gOverlay52Objects[];

/* This exact six-overlay Mickey family has no exact DKR v77/v80 or JFG donor. */
void overlay52PatchIndices(OverlayPatchIndexEntry *entry) {
    while (entry->first != 0) {
        entry->first = (s32) gOverlay52Objects[entry->first];
        if (entry->second != 0) {
            entry->second = (s32) gOverlay52Objects[entry->second];
        }
        entry++;
    }
}
