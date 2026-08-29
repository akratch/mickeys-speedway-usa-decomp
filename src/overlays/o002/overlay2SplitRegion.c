#include "PR/ultratypes.h"

typedef struct Overlay2Region {
    s32 boundaryAxis;
    f32 boundaryValue;
    struct Overlay2Region *side1;
    struct Overlay2Region *side0;
    u16 start;
    u16 count;
} Overlay2Region;

extern Overlay2Region *D_4;
extern s32 D_24;

extern s32 func_overlay_002_F0000000_1856DF8(Overlay2Region *region);
extern void func_overlay_002_F00006E0_18574D8(Overlay2Region *region);
extern void func_overlay_002_F000049C_1857294(Overlay2Region *input,
                                               Overlay2Region *output,
                                               s32 wantedSide);

#define gOverlay2Regions D_4
#define gOverlay2RegionCount D_24
#define overlay2ChooseBoundary func_overlay_002_F00006E0_18574D8
#define overlay2ClipLines func_overlay_002_F000049C_1857294

/* Exact 72-word C. IDO 5.3's fidelity-clean as1 scheduler trace showed the
 * prologue's ready nodes split between the function and loop-header source
 * lines. Keeping those token-equivalent headers on one physical line produces
 * the retail order; all nine runtime relocation tuples and identities agree. */
void overlay2SplitRegion(Overlay2Region *previous, Overlay2Region *region) { for (;;) {
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
