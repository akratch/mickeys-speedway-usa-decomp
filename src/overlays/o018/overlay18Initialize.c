#include "PR/ultratypes.h"

extern void overlay18ResetReloc(void);
extern void *overlay18CreateReloc(s32, s32);
extern void overlay18UseReloc(void *);
extern void overlay18FinishReloc(void);
extern void *gOverlay18Handle;
extern void *gOverlay18HandleSlot;
extern u32 gOverlay18State;

/* DKR v77/v80 and JFG have no exact donor for this initialization sequence. */
void overlay18Initialize(void) {
    void *handle;
    register void **slot;

    overlay18ResetReloc();
    gOverlay18Handle = NULL;
    handle = overlay18CreateReloc(0xF4, 0x8F);
    slot = &gOverlay18HandleSlot;
    *slot = handle;
    overlay18UseReloc(*slot);
    overlay18FinishReloc();
    gOverlay18State = 0x10000000;
}
