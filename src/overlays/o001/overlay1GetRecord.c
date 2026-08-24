#include "PR/ultratypes.h"

typedef struct Overlay1Record {
    u8 bytes[0x94];
} Overlay1Record;

/* Pinned DKR v77/v80 and JFG object scans found no exact donor. */
extern Overlay1Record *gOverlay1Start;
extern s32 gOverlay1EntryCount;

Overlay1Record *overlay1GetRecord(s32 index) {
    if (gOverlay1Start != NULL && index < gOverlay1EntryCount) {
        return &gOverlay1Start[index];
    }
    return NULL;
}
