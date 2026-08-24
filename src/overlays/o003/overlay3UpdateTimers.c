#include "PR/ultratypes.h"

typedef struct Overlay3TimedEntry {
    s32 value;
    s32 timer;
} Overlay3TimedEntry;

extern Overlay3TimedEntry gOverlay3EntriesEnd[32];

/* DKR v77/v80 and JFG contain no exact donor for this timer sweep. */
s32 overlay3UpdateTimers(s32 updateRate, Overlay3TimedEntry *groupEntries) {
    s32 result;
    s32 group;
    s32 remaining;
    Overlay3TimedEntry *entry;

    group = 9;
    groupEntries = gOverlay3EntriesEnd;
    do {
        remaining = 31;
        entry = &groupEntries[31];
        do {
            if (entry->timer != 0) {
                if (updateRate < entry->timer) {
                    entry->timer -= updateRate;
                } else {
                    entry->value = 0;
                    entry->timer = 0;
                }
            }
            entry--;
        } while (result = remaining--);
        groupEntries -= 32;
    } while (result = group--);
    return result;
}
