#include "PR/ultratypes.h"

/* DKR v77/v80 and JFG exact-object scans are negative for this timer sweep. */
typedef struct Overlay3TimedEntry {
    void *object;
    s32 timer;
} Overlay3TimedEntry;

extern Overlay3TimedEntry gOverlay3Entries[][32];

void overlay3UpdateTimedEntries(s32 amount) {
    Overlay3TimedEntry *group;
    Overlay3TimedEntry *entry;
    s32 groupRemaining;
    s32 remaining;
    register s32 timer;

    timer = 0;
    groupRemaining = 9; group = gOverlay3Entries[9];
    do {
        remaining = 31;
        entry = &group[31];
        do {
            timer = entry->timer;
            if (timer != 0) {
                if (amount < timer) {
                    entry->timer = timer - amount;
                } else {
                    entry->object = NULL;
                    entry->timer = 0;
                }
            }
            entry--;
        } while (remaining--);
        group -= 32;
    } while (groupRemaining--);
}
