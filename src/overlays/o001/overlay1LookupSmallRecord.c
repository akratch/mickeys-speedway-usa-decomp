#include "PR/ultratypes.h"

typedef struct Overlay1SmallRecord {
    u8 bytes[0x1C];
} Overlay1SmallRecord;

/* Pinned DKR v77/v80 and JFG object scans found no exact donor. */
extern Overlay1SmallRecord *gOverlay1SmallRecords;

void *overlay1LookupSmallRecord(s32 index) {
    void *result;
    s32 offset;

    offset = index * 7;
    result = NULL;
    if (gOverlay1SmallRecords == NULL) {
        goto null_result;
    }
    result = (Overlay1SmallRecord *)((u8 *)gOverlay1SmallRecords + offset * 4);
    return result;
null_result:
    return result;
}
