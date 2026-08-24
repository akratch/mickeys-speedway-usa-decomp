#include "PR/ultratypes.h"

typedef struct Overlay101Entry {
    u8 pad00[4];
    void *value4;
    u8 pad08[0x28];
} Overlay101Entry;

extern s32 gOverlay101EntryCount;
extern Overlay101Entry gOverlay101Entries[];

/* Pinned DKR v77/v80 and JFG scans found no exact donor for this pool scan. */
Overlay101Entry *overlay101AllocateEntry(void) {
    Overlay101Entry *entry;
    Overlay101Entry *result;
    s32 count;
    register s32 remaining;

    count = gOverlay101EntryCount;
    entry = gOverlay101Entries;
    remaining = 0x27;
    if (gOverlay101EntryCount < 0x28) {
        do {
            if (entry->value4 == NULL) {
                gOverlay101EntryCount = count + 1;
                return entry;
            }
            entry++;
        } while (remaining--);
    }
    result = NULL;
    return result;
}
