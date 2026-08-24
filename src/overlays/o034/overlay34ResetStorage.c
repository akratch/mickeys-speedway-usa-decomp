#include "PR/ultratypes.h"

typedef struct Overlay34Record Overlay34Record;

extern Overlay34Record *gOverlay34Records;
extern Overlay34Record **gOverlay34Pointers;
extern s32 gOverlay34Count;
extern s32 gOverlay34ActiveCount;
extern void overlay34RemoveRecord(Overlay34Record *record);
extern void func_8002B768(void *address);

void overlay34ResetStorage(void) {
    if (gOverlay34Pointers != NULL) {
        if (gOverlay34ActiveCount != 0) {
            do {
                overlay34RemoveRecord(*gOverlay34Pointers);
            } while (gOverlay34ActiveCount != 0);
        }
        func_8002B768(gOverlay34Records);
        func_8002B768(gOverlay34Pointers);
    }
    gOverlay34Count = 0;
    gOverlay34ActiveCount = 0;
    gOverlay34Records = NULL;
    gOverlay34Pointers = NULL;
}
