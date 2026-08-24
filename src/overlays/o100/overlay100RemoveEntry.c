#include "PR/ultratypes.h"

extern s32 gOverlay100EntryCount;
extern void *gOverlay100Entries[];
extern void overlay100FinalizeReloc(void *entry);

/*
 * DKR v77/v80 and JFG contain generic array-compaction relatives, but no
 * exact object donor for this exact registry removal routine.
 */
void overlay100RemoveEntry(void *entry) {
    s32 i;

    if (gOverlay100EntryCount > 0) {
        for (i = 0; i < gOverlay100EntryCount; i++) {
            if (gOverlay100Entries[i] == entry) {
                gOverlay100EntryCount--;
                break;
            }
        }

        while (i < gOverlay100EntryCount) {
            gOverlay100Entries[i] = gOverlay100Entries[i + 1];
            i++;
        }
    }
    overlay100FinalizeReloc(entry);
}
