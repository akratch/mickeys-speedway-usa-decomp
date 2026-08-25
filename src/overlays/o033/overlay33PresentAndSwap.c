#include "PR/ultratypes.h"

extern volatile s32 gOverlay33BufferIndex;
extern u8 gOverlay33CommandStorage[];
extern void *gOverlay33DisplayLists[];
extern void *gOverlay33BufferRefs[];
extern void *gOverlay33ActiveBuffer;
extern s32 gOverlay33Ready;

extern void overlay33FlushReloc(void);
extern void overlay33SubmitReloc(void *commands, void *displayList, s32 mode,
                                 void *buffer);

/*
 * Plateau (10 source-shape attempts): the default flags retain the exact
 * 39-instruction size and 24-byte frame, with 21 positional words differing
 * and the first mismatch at +0x10.  Control flow and scheduling agree; the
 * blocker is the complete private temporary-register web.  The strong JFG
 * refractTick skeleton hit is assembly-only and supplies no donor C body.
 * A current-lane full flag resweep and three typed volatile-pointer lifetime
 * variants compiled identically: 21 differing words, first at +0x10.  That
 * independently exhausts the qualifier/alias hypothesis without register
 * order guessing.
 */
#ifdef NON_MATCHING
void overlay33PresentAndSwap(void) {
    s32 index;

    overlay33FlushReloc();
    index = gOverlay33BufferIndex;
    overlay33SubmitReloc(&gOverlay33CommandStorage[index * 0xC00],
                         gOverlay33DisplayLists[index], 4,
                         gOverlay33BufferRefs[index]);
    index = gOverlay33BufferIndex ^ 1;
    gOverlay33BufferIndex = index;
    gOverlay33ActiveBuffer = gOverlay33BufferRefs[index];
    gOverlay33Ready = 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o033/overlay33PresentAndSwap/func_overlay_033_F000066C_1880E54.s")
#endif
