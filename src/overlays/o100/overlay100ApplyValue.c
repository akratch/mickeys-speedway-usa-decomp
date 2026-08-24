#include "ultra64.h"

extern s32 gOverlay100Count;
extern void *gOverlay100Entries[];
extern void overlay100ApplyValueReloc(void *entry, s32 value);

/* DKR v77/v80 and JFG contain no exact fixed-list apply donor. */
void overlay100ApplyValue(s32 value) {
    s32 count;
    count = gOverlay100Count;
    if (count != 0) {
        while (count--) {
            overlay100ApplyValueReloc(gOverlay100Entries[count], value);
        }
    }
}
