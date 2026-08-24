#include "PR/ultratypes.h"

typedef struct Overlay101GlobalPairEntry {
    void *output;
    void *owner;
    s32 startX;
    s32 startY;
    u8 pad10[8];
    s32 endX;
    s32 endY;
    u8 pad20[8];
    s32 elapsed;
    s32 duration;
} Overlay101GlobalPairEntry;

extern s32 gOverlay101EntryCount;
extern s32 gOverlay101GlobalX;
extern s32 gOverlay101GlobalY;

void overlay101UpdateGlobalPair(Overlay101GlobalPairEntry *entry, s32 step) {
    s32 elapsed;

    elapsed = (entry->elapsed += step);
    if (elapsed >= entry->duration) {
        gOverlay101GlobalX = entry->endX;
        gOverlay101GlobalY = entry->endY;
        entry->owner = NULL;
        gOverlay101EntryCount--;
        return;
    }
    gOverlay101GlobalX = entry->startX +
        ((entry->endX - entry->startX) * elapsed) / entry->duration;
    gOverlay101GlobalY = entry->startY +
        ((entry->endY - entry->startY) * entry->elapsed) / entry->duration;
}
