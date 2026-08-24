#include "PR/ultratypes.h"

extern void *gOverlay50ResourceA;
extern void *gOverlay50ResourceB;
extern u8 gOverlay50Storage;
extern s8 gOverlay50Slot;

extern void overlay50ReleaseResourceReloc(void *resource);
extern void overlay50ReleaseStorageReloc(void *storage);
extern void overlay50ResetResourcesReloc(void);
extern void overlay50ReleaseSlotReloc(s32 slot);

/* DKR v77/v80 and JFG contain no exact donor for this resource teardown. */
void overlay50Cleanup(void) {
    if (gOverlay50ResourceA != NULL) {
        overlay50ReleaseResourceReloc(gOverlay50ResourceA);
    }
    overlay50ReleaseStorageReloc(&gOverlay50Storage);
    overlay50ResetResourcesReloc();
    if (gOverlay50ResourceB != NULL) {
        overlay50ReleaseResourceReloc(gOverlay50ResourceB);
    }
    if (gOverlay50Slot != -1) {
        overlay50ReleaseSlotReloc(gOverlay50Slot);
        gOverlay50Slot = -1;
    }
}
