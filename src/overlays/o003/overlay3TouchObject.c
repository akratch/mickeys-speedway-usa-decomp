#include "PR/ultratypes.h"

typedef struct Overlay3TouchEntry {
    void *object;
    s32 timer;
} Overlay3TouchEntry;

extern void *gOverlay3Objects[];
extern Overlay3TouchEntry gOverlay3TouchEntries[][32];

/* DKR v77/v80 and JFG contain no exact donor for this fixed object registry. */
void overlay3TouchObject(s32 group, s32 remaining) {
    void *object = gOverlay3Objects[group];
    Overlay3TouchEntry *base;
    Overlay3TouchEntry *entry;

    remaining = 31;
    if (object == 0) {
        return;
    }
    base = gOverlay3TouchEntries[group];
    entry = &base[remaining];
    do {
        if (entry->object == object) {
            entry->timer = 300;
            return;
        }
        entry--;
    } while (remaining--);

    remaining = 31;
    entry = base + 31;
    do {
        if (entry->object == 0) {
            entry->object = object;
            entry->timer = 300;
            return;
        }
        entry--;
    } while (remaining--);
}
