#include "PR/ultratypes.h"

typedef struct Overlay27Resource {
    u8 state;
    u8 pad01[0x13];
    f32 value14;
} Overlay27Resource;

typedef struct Overlay27Object {
    u8 pad00[0x64];
    Overlay27Resource *resource;
} Overlay27Object;

/* Fresh pinned DKR v77/v80 and JFG object scans found no exact donor. */
s32 overlay27CanUse(Overlay27Object *object) {
    if (object != NULL) {
        if (object->resource->state != 4 || object->resource->value14 > 0.0f) {
            return 1;
        }
    }
    return 0;
}
