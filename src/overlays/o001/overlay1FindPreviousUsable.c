#include "PR/ultratypes.h"

typedef struct Overlay1RingRecord {
    u8 pad00[0x10];
    u16 flags;
    u8 pad12[0x82];
} Overlay1RingRecord;

extern Overlay1RingRecord *gOverlay1Start;
extern s32 gOverlay1EntryCount;

/* The pinned DKR v77/v80 and JFG object scans contain no exact donor. */
Overlay1RingRecord *overlay1FindPreviousUsable(s32 index, s32 *selectedIndex) {
    s32 count;
    s32 remaining;
    s32 wrapCount;
    Overlay1RingRecord *record;
    Overlay1RingRecord *records;
    u16 flags;

    records = gOverlay1Start;
    if (records != NULL) {
        count = gOverlay1EntryCount;
        if (index < count) {
            remaining = count;
            wrapCount = count;
            if (remaining != 0) {
                remaining--;
                do {
                    index--;
                    if (index < 0) {
                        index = wrapCount - 1;
                    }
                    record = &records[index];
                    flags = record->flags;
                    if (!(flags & 4) && !(flags & 8)) {
                        *selectedIndex = index;
                        return record;
                    }
                } while (remaining--);
            }
        }
    }
    *selectedIndex = -1;
    return NULL;
}
