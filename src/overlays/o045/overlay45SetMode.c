#include "overlays/o045/resource_descriptor.h"

/*
 * Overlay 45 +0x1BE0. Fresh DKR v77/v80 searches for a null-guarded +0x1D
 * descriptor byte setter were negative.
 */
void overlay45SetMode(Overlay45ResourceDescriptor *descriptor, s32 mode) {
    if (descriptor != NULL) {
        descriptor->mode = mode;
    }
}
