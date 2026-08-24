#include "PR/ultratypes.h"

/* Pinned DKR v77/v80 and JFG scans classify overlay 1 as no donor. */
extern void *gOverlay1DistanceObject;
extern u8 gOverlay1SelectedIndex;
extern void **overlay1GetSelectionReloc(s32 *count);
extern f32 overlay1DistanceReloc(void *first, void *second);

f32 overlay1DistanceFromSelected(void *object) {
    s32 count;
    void **objects;
    volatile s32 reservation[2];

    if (object == gOverlay1DistanceObject) {
        objects = overlay1GetSelectionReloc(&count);
        if (count >= 2) {
            return overlay1DistanceReloc(object, objects[gOverlay1SelectedIndex]);
        }
    }
    return 0.0f;
}
