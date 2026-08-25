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
 * Plateau: the best -O2/-mips2 body is exact-size and differs in 9/72
 * relocation-masked words, first at +0xC; every word from +0x38 onward is
 * aligned, leaving only the callee-save/global-address prologue schedule.
 * Reversing the two equality spellings removed four real differences. A
 * five-minute permuter batch improved its internal score only by inventing a
 * repeated null guard and a dead assignment, so that candidate was rejected.
 */
#ifdef NON_MATCHING
void overlay2SplitRegion(Overlay2Region *previous, Overlay2Region *region) {
    Overlay2Region *current;
    Overlay2Region *parent;

    current = region;
    parent = previous;
    for (;;) {
        if (func_overlay_002_F0000000_1856DF8(current) != 0) {
            break;
        }
        overlay2ChooseBoundary(current);
        if ((parent != NULL) &&
            (parent->boundaryAxis == current->boundaryAxis) &&
            (parent->boundaryValue == current->boundaryValue)) {
            break;
        }

        current->side1 = &gOverlay2Regions[gOverlay2RegionCount];
        gOverlay2RegionCount++;
        overlay2ClipLines(current, current->side1, 1);
        current->side0 = &gOverlay2Regions[gOverlay2RegionCount];
        gOverlay2RegionCount++;
        overlay2ClipLines(current, current->side0, 0);
        overlay2SplitRegion(current, current->side1);
        parent = current;
        current = current->side0;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o002/overlay2SplitRegion/func_overlay_002_F0000B70_1857968.s")
#endif
