#include "PR/ultratypes.h"

extern s32 gOverlay1EntryCount;

s32 overlay1NextIndex(s32 index) {
    index++;
    if (index >= gOverlay1EntryCount) {
        index = 0;
    }
    return index;
}
