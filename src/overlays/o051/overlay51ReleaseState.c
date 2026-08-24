#include "PR/ultratypes.h"

extern u8 gOverlay51InlineResource[];
extern s8 gOverlay51Index;
extern void overlay51ReleaseReloc(void *resource);
extern void overlay51FinalizeReloc(void);
extern void overlay51ReleaseIndexReloc(s32 index);

/* DKR v77/v80 and JFG checks found only generic resource cleanup. */
void overlay51ReleaseState(void) {
    s32 index;

    overlay51ReleaseReloc(gOverlay51InlineResource);
    overlay51FinalizeReloc();
    index = gOverlay51Index;
    if (index != -1) {
        overlay51ReleaseIndexReloc(index);
        gOverlay51Index = -1;
    }
}
