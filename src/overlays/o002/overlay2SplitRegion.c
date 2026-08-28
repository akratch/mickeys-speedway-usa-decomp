#include "PR/ultratypes.h"

typedef struct Overlay2Region {
    s32 boundaryAxis;
    f32 boundaryValue;
    struct Overlay2Region *side1;
    struct Overlay2Region *side0;
    u16 start;
    u16 count;
} Overlay2Region;

extern Overlay2Region *gOverlay2Regions;
extern s32 gOverlay2RegionCount;

extern s32 func_overlay_002_F0000000_1856DF8(Overlay2Region *region);
extern void overlay2ChooseBoundary(Overlay2Region *region);
extern void overlay2ClipLines(Overlay2Region *input, Overlay2Region *output,
                              s32 wantedSide);

/*
 * Plateau (2026-08-25): the best -O2/-mips2 body is exact-size and differs
 * in 7/72
 * relocation-masked words, first at +0xC; every word from +0x38 onward is
 * aligned, leaving only the callee-save/global-address prologue schedule.
 * Reusing the two parameters as the loop state removed two differences;
 * explicit global-address aliases regressed the full body. Reversing the two
 * equality spellings previously removed four real differences. A five-minute
 * permuter batch improved its internal score only by inventing a repeated null
 * guard and a dead assignment, so that candidate was rejected.
 * Fresh lane recheck: all 119 flag combinations still bottom out at the same
 * 7/72 words and +0xC first mismatch. Pairing the allocation/increment source
 * lines, spelling the m2c compound condition directly, and compiling through
 * the raw D_4/D_24 symbols all produced the same residual. This remains a
 * late prologue-scheduling plateau with no remaining structural lead.
 * Tier-2 trace revisit (2026-08-28): proc 0 retained five natural allocator
 * webs (s0-s4), but trace-stack-homes reported no producer-emitted virtual or
 * final-home fields. A same-line tail-state reassignment probe preserved the
 * exact 0x120-byte/frame-0x30 body and reproduced 7/72 words with first
 * mismatch +0xC and the identical prologue reorder. The grounded family is
 * parked pending new scheduler evidence.
 */
#ifdef NON_MATCHING
void overlay2SplitRegion(Overlay2Region *previous, Overlay2Region *region) {
    for (;;) {
        if (func_overlay_002_F0000000_1856DF8(region) != 0) {
            break;
        }
        overlay2ChooseBoundary(region);
        if ((previous != NULL) &&
            (previous->boundaryAxis == region->boundaryAxis) &&
            (previous->boundaryValue == region->boundaryValue)) {
            break;
        }

        region->side1 = &gOverlay2Regions[gOverlay2RegionCount];
        gOverlay2RegionCount++;
        overlay2ClipLines(region, region->side1, 1);
        region->side0 = &gOverlay2Regions[gOverlay2RegionCount];
        gOverlay2RegionCount++;
        overlay2ClipLines(region, region->side0, 0);
        overlay2SplitRegion(region, region->side1);
        previous = region;
        region = region->side0;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o002/overlay2SplitRegion/func_overlay_002_F0000B70_1857968.s")
#endif
