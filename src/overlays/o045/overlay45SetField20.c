#include "overlays/o045/resource_descriptor.h"

/* DKR v77/v80 checks found only generic null-guarded field setters. */
void overlay45SetField20(Overlay45ResourceDescriptor *descriptor, s32 value) {
    if (descriptor != NULL) {
        descriptor->unk20 = value;
    }
}
