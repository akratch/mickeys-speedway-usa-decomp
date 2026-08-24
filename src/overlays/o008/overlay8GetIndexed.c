#include "PR/ultratypes.h"

typedef struct Overlay8IndexedObject {
    u8 pad0;
    s8 index;
} Overlay8IndexedObject;

extern u8 gOverlay8IndexMode;
extern void *gOverlay8Primary[];
extern void *gOverlay8Secondary[];

/* DKR v77/v80 and JFG checks found no exact donor for this indexed selector. */
void *overlay8GetIndexed(Overlay8IndexedObject *object) {
    s32 index = object->index;
    void *result;

    if (index < 0 || index >= 10) {
        index = 0;
    }
    if (gOverlay8IndexMode == 0) {
        result = gOverlay8Primary[index];
    } else {
        result = gOverlay8Secondary[index];
    }
    return result;
}
