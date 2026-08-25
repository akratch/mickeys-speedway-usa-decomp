#include "PR/ultratypes.h"

extern u8 gOverlay33Initialized;
extern s32 gOverlay33StateA;
extern s32 gOverlay33StateB;
extern s32 gOverlay33Mode;
extern volatile s32 gOverlay33Allocation;
extern s32 gOverlay33InitializeContext;
extern s32 gOverlay33AlignedBuffers[];
extern s32 gOverlay33BufferIndex;
extern s32 gOverlay33ActiveBuffer;

extern void overlay33GetDimensionsReloc(s32 *width, s32 *height);
extern s32 overlay33AllocateReloc(s32 size, s32 tag);
extern s32 overlay33InitializeBufferReloc(s32 *context, s32 *status, s32 mode);
extern void overlay33AllocationFailedReloc(void);

/*
 * Plateau (10 source-shape attempts): the best candidate has the exact
 * 81-instruction size and 56-byte frame, with 14 positional words differing
 * and the first mismatch at +0x74.  The remaining blocker is the allocation
 * alignment delay-slot schedule and its v0/v1 temporary register web.
 */
#ifdef NON_MATCHING
void overlay33InitializeBuffers(void) {
    s32 width;
    s32 height;
    volatile s32 unused;
    s32 status;
    s32 allocation;
    s32 original;

    status = 0;
    if (gOverlay33Initialized == 0) {
        gOverlay33StateA = 0;
        gOverlay33StateB = 0;
        gOverlay33Mode = 2;
        gOverlay33Allocation = 0;
        overlay33GetDimensionsReloc(&width, &height);
        allocation = overlay33AllocateReloc((width * height * 4) + 0x40,
                                             0x87);
        gOverlay33Allocation = allocation;
        if (allocation != 0) {
            original = allocation;
            if (allocation & 0x3F) {
                allocation = (allocation & ~0x3F) + 0x40;
            } else {
                allocation = original;
            }
            gOverlay33AlignedBuffers[0] = allocation;
            gOverlay33AlignedBuffers[1] = allocation + (width * height * 2);
            gOverlay33ActiveBuffer =
                gOverlay33AlignedBuffers[gOverlay33BufferIndex];
            if (overlay33InitializeBufferReloc(&gOverlay33InitializeContext,
                                                &status, 0) == 0) {
                do {
                } while (overlay33InitializeBufferReloc(
                             &gOverlay33InitializeContext, &status, 0) == 0);
            }
            gOverlay33Initialized = 1;
        } else {
            overlay33AllocationFailedReloc();
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o033/overlay33InitializeBuffers/func_overlay_033_F0000000_18807E8.s")
#endif
