#include "PR/ultratypes.h"

/* DKR thread3_main has the semantic double-display-list lifecycle, not this exact allocator sequence. */
extern void overlay42PrepareReloc(void);
extern void *overlay42AllocReloc(s32 size, s32 tag);
extern void *gOverlay42Buffer0;
extern void *gOverlay42Buffer1;
extern void *gOverlay42Buffer2;
extern s32 gOverlay42Ready;

void overlay42Init(void) {
    overlay42PrepareReloc();
    gOverlay42Buffer0 = overlay42AllocReloc(0x2000, 0x87);
    gOverlay42Buffer1 = overlay42AllocReloc(0x2000, 0x87);
    gOverlay42Buffer2 = 0;
    gOverlay42Ready = 1;
}
