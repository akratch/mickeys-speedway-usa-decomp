#include "PR/ultratypes.h"

typedef struct Overlay41ScaledObject {
    u8 pad00[0x1C];
    f32 scale;
} Overlay41ScaledObject;

/* Fresh pinned DKR v77/v80 and JFG scans found no exact donor for this leaf. */
extern Overlay41ScaledObject **gOverlay41Objects;

s32 overlay41IsUnitScale(s32 index) {
    Overlay41ScaledObject *object;

    object = gOverlay41Objects[index];
    if (object != NULL && object->scale == 1.0f) {
        return 1;
    }
    return 0;
}
