#include "PR/ultratypes.h"

typedef struct Overlay1ReleaseRecord {
    u8 pad00[0x14];
    void *resource;
    u8 pad18[4];
} Overlay1ReleaseRecord;

extern s32 gOverlay1ReleaseRecordCount;
extern Overlay1ReleaseRecord *gOverlay1ReleaseRecords;
extern void *gOverlay1ReleaseSecondary;
extern void *gOverlay1ReleaseFinal;
extern void overlay1ReleaseReloc(void *resource);

/* DKR v77/v80 and JFG have generic teardown loops, but no exact donor. */
void overlay1ReleaseRecords(void) {
    s32 remaining;
    Overlay1ReleaseRecord *record;

    if (gOverlay1ReleaseRecordCount != 0) {
        remaining = gOverlay1ReleaseRecordCount - 1;
        record = &gOverlay1ReleaseRecords[1];
        while (remaining-- > 0) {
            if (record->resource != NULL) {
                overlay1ReleaseReloc(record->resource);
            }
            record++;
        }
        overlay1ReleaseReloc(gOverlay1ReleaseRecords);
        overlay1ReleaseReloc(gOverlay1ReleaseSecondary);
        gOverlay1ReleaseRecords = NULL;
        gOverlay1ReleaseSecondary = NULL;
    }
    if (gOverlay1ReleaseFinal != NULL) {
        overlay1ReleaseReloc(gOverlay1ReleaseFinal);
    }
}
