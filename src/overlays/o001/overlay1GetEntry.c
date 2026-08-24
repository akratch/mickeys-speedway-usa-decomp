#include "PR/ultratypes.h"

typedef struct Overlay1Entry {
    u8 bytes[0x1C];
} Overlay1Entry;

extern Overlay1Entry *gOverlay1Entries;

/* The pinned DKR v77/v80 and JFG object scans contain no exact donor. */
Overlay1Entry *overlay1GetEntry(s32 index) {
    Overlay1Entry *result;
    Overlay1Entry *entries;

    entries = gOverlay1Entries;
    result = NULL; if (entries != NULL) result = &entries[index];
    return result;
}
