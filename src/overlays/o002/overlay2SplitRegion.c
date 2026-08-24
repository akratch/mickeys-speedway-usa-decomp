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
extern s32 overlay2ClipLines(Overlay2Region *input, Overlay2Region *output,
                             s32 wantedSide);

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
            (current->boundaryAxis == parent->boundaryAxis) &&
            (current->boundaryValue == parent->boundaryValue)) {
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
