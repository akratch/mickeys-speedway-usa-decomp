#include "ultra64.h"

extern s32 gOverlay100Count;
extern void *gOverlay100Entries[];
extern void overlay100ReleaseReloc(void *entry);

/* DKR v77/v80 and JFG contain only generic object-list cleanup relatives. */
void overlay100ReleaseAll(void) {
    s32 count;

    count = gOverlay100Count;
    if (count != 0) {
        while (count--) {
            overlay100ReleaseReloc(gOverlay100Entries[count]);
        }
    }
}
