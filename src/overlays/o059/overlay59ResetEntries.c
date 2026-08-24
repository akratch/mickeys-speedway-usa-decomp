#include "PR/ultratypes.h"

typedef struct Overlay59Entry {
    f32 value0;
    f32 value4;
    u8 pad08[0x10];
    s16 state18;
    u8 pad1A[4];
    s16 state1E;
    s32 state20;
    s32 values[8];
} Overlay59Entry;

/* Pinned DKR v77/v80 and JFG object scans found no exact donor. */
extern Overlay59Entry gOverlay59Entries[];
extern Overlay59Entry gOverlay59EntriesEnd[];

void overlay59ResetEntries(void) {
    Overlay59Entry *entry;
    Overlay59Entry *cursor;
    Overlay59Entry *end;
    s32 i;
    s32 limit;

    i = 0;
    cursor = entry = gOverlay59Entries; limit = 8; end = gOverlay59EntriesEnd;
    do {
        entry->value0 = 0.0f;
        entry->value4 = 1.0f;
        entry->state18 = 0;
        entry->state1E = 0;
        entry->state20 = 0;
        i = 0;
        cursor = entry;
        do {
            cursor->values[0] = 0;
            cursor->values[1] = 0;
            cursor->values[2] = 0;
            cursor->values[3] = 0;
            i += 4;
            cursor = (Overlay59Entry *)((u8 *)cursor + 0x10);
        } while (i != limit);
        entry++;
        cursor = entry;
        i = 0;
    } while (entry != end);
}
