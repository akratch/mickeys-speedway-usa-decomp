#include "PR/ultratypes.h"

typedef struct Overlay3Entry {
    s32 value;
    s32 unused;
} Overlay3Entry;

typedef struct Overlay3Resource {
    u8 pad0;
    s8 group;
} Overlay3Resource;

typedef struct Overlay3Owner {
    u8 pad00[0x64];
    Overlay3Resource *resource;
} Overlay3Owner;

/* Pinned DKR v77/v80 and JFG object scans found no exact donor. */
extern Overlay3Entry gOverlay3Entries[][32];

s32 overlay3ContainsValue(Overlay3Owner *owner, s32 value) {
    Overlay3Entry *entry;
    s32 group;
    s32 previous;
    s32 remaining;

    group = owner->resource->group;
    remaining = 31;
    entry = &gOverlay3Entries[group][31];
    do {
        if (value == entry->value) {
            return 1;
        }
        previous = remaining--;
        entry--;
    } while (previous);
    return 0;
}
