#include "PR/ultratypes.h"

/* Fresh pinned DKR v77/v80 and JFG scans found no exact donor for this setup leaf. */
extern s32 overlay51CreateReloc();
extern void overlay51PrepareReloc();
extern u8 gOverlay51Resource0[];
extern u8 gOverlay51Resource18[];
extern u8 gOverlay51ResourceBC[];
extern u8 gOverlay51Resource1C[];
extern f32 gOverlay51InitialValue;
extern s8 gOverlay51Mode;
extern s16 gOverlay51Handle;

void overlay51Initialize(void) {
    overlay51CreateReloc(gOverlay51Resource0);
    overlay51CreateReloc(gOverlay51Resource18);
    overlay51CreateReloc(4);
    overlay51CreateReloc(11);
    overlay51PrepareReloc(gOverlay51ResourceBC);
    overlay51PrepareReloc(gOverlay51Resource1C);
    gOverlay51InitialValue = -80.0f;
    overlay51CreateReloc();
    gOverlay51Mode = -1;
    gOverlay51Handle = overlay51CreateReloc();
}
