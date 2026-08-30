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
 * Plateau (2026-08-30): using the preserved pre-alignment value for both the
 * test and mask improves the exact-sized 81-word, 0x38-frame body from 14 to
 * 6 relocation-masked differences. The first codegen mismatch is +0x74; the
 * remaining blocker is a store/branch/copy scheduling cluster and one
 * commutative addition order. All 25 fallback relocation sites align by
 * offset/type, but their LOCAL/data identities remain unauthenticated.
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
            if (original & 0x3F) {
                allocation = (original & ~0x3F) + 0x40;
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

/* PLATEAU-HANDOFF:overlay33InitializeBuffers:start
 * symbol: overlay33InitializeBuffers
 * score: 75/81 words
 * frame: 0x38
 * relocations: 25
 * first-mismatch: +0x74
 * summary: Exact allocator lanes; six positional differences remain: store/branch/copy at +0x74 and one add order. Fallback has 25 sites, zero identities.
 * PLATEAU-HANDOFF:overlay33InitializeBuffers:end
 */
