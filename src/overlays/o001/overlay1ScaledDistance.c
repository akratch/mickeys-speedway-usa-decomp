#include "PR/ultratypes.h"

/* Pinned DKR v77/v80 and JFG scans classify overlay 1 as no donor. */
extern f32 gOverlay1DistanceScale;
extern f32 overlay1DistanceReloc(void *first, void *second);

f32 overlay1ScaledDistance(void *first, void *second) {
    if (first == second || first == NULL || second == NULL) {
        return 0.0f;
    }
    return overlay1DistanceReloc(second, first) * gOverlay1DistanceScale;
}
