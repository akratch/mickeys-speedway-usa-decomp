#include "overlays/overlay_045.h"

/*
 * Overlay 45 matched-C tail after the hand-written middle. Exact DKR v77/v80
 * and JFG scans are negative beyond generic null-guarded field setters.
 */

void overlay45SetMode(Overlay45ResourceDescriptor *descriptor, s32 mode) {
    if (descriptor != NULL) {
        descriptor->mode = mode;
    }
}

void overlay45SetField22(Overlay45ResourceDescriptor *descriptor, s32 value) {
    if (descriptor != NULL) {
        descriptor->unk22 = value;
    }
}

void overlay45SetField20(Overlay45ResourceDescriptor *descriptor, s32 value) {
    if (descriptor != NULL) {
        descriptor->unk20 = value;
    }
}
