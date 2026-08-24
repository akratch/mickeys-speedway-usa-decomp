#include "PR/ultratypes.h"

typedef struct Overlay59Value {
    u8 first;
    s8 second;
} Overlay59Value;

typedef struct Overlay59Entry {
    u8 pad00[8];
    Overlay59Value values[8];
    s16 count;
    u8 pad1A[0x2A];
} Overlay59Entry;

extern Overlay59Entry gOverlay59Entries[4];
extern u8 *gOverlay59ValueTables[11];

/* DKR v77/v80 and JFG contain no exact donor for this value append helper. */
void overlay59AppendValue(s32 index, s32 tableIndex, s32 offset, s32 mode) {
    Overlay59Entry *entry;
    s32 count;

    if (index < 0 || index >= 4) {
        return;
    }
    if (tableIndex < 0 || tableIndex >= 11) {
        tableIndex = 0;
    }
    if (mode < 0 || mode >= 6) {
        mode = 5;
    }
    entry = &gOverlay59Entries[index];
    offset = gOverlay59ValueTables[tableIndex][offset];
    count = entry->count;
    if (count != 0 && offset == entry->values[count - 1].first) {
        return;
    }
    if (count < 8) {
        entry->values[count].first = offset;
        entry->values[entry->count].second = mode;
        entry->count++;
    }
}
