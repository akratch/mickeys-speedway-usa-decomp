#include "overlays/patch_indices.h"

extern void *gOverlay50Objects[];

/* This exact six-overlay Mickey family has no exact DKR v77/v80 or JFG donor. */
void overlay50PatchIndices(OverlayPatchIndexEntry *entry) {
    while (entry->first != 0) {
        entry->first = (s32) gOverlay50Objects[entry->first];
        if (entry->second != 0) {
            entry->second = (s32) gOverlay50Objects[entry->second];
        }
        entry++;
    }
}
