#include "PR/ultratypes.h"

/* Pinned DKR v77/v80 and JFG scans classify overlay 1 as no donor. */
extern void *gOverlay1DistanceObject;
extern f32 overlay1DistanceReloc(void *first, void *second);

f32 overlay1DistanceFromCurrent(void *other) {
    void *current;

    current = gOverlay1DistanceObject;
    if (current != NULL) {
        return overlay1DistanceReloc(current, other);
    }
    return 0.0f;
}
