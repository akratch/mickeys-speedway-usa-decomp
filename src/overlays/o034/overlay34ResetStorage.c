#include "PR/ultratypes.h"

typedef struct Overlay34Record Overlay34Record;

extern Overlay34Record *gOverlay34Records;
extern Overlay34Record **gOverlay34Pointers;
extern s32 gOverlay34Count;
extern s32 gOverlay34ActiveCount;
extern void overlay34RemoveRecord(Overlay34Record *record);
extern void mmFree(void *address);

void overlay34ResetStorage(void) {
    if (gOverlay34Pointers != NULL) {
        if (gOverlay34ActiveCount != 0) {
            do {
                overlay34RemoveRecord(*gOverlay34Pointers);
            } while (gOverlay34ActiveCount != 0);
        }
        mmFree(gOverlay34Records);
        mmFree(gOverlay34Pointers);
    }
    gOverlay34Count = 0;
    gOverlay34ActiveCount = 0;
    gOverlay34Records = NULL;
    gOverlay34Pointers = NULL;
}
