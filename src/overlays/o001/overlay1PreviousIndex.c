#include "PR/ultratypes.h"

extern s32 gOverlay1EntryCount;

s32 overlay1PreviousIndex(s32 index) {
    index--;
    if (index < 0) {
        index = gOverlay1EntryCount - 1;
    }
    return index;
}
