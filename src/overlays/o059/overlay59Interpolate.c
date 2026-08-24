#include "PR/ultratypes.h"

typedef struct Overlay59Entry {
    f32 scale0;
    f32 scale4;
    u8 pad08[0x18];
    void *owner;
    u8 pad24[0x20];
} Overlay59Entry;

extern Overlay59Entry gOverlay59Entries[4];

/* DKR v77/v80 and JFG contain no exact donor for this interpolation helper. */
s32 overlay59Interpolate(s32 index, s32 x, s32 y, s32 targetX, s32 targetY,
                         s32 *outX, s32 *outY, s32 useFirstScale) {
    Overlay59Entry *entry;
    f32 deltaX;
    f32 deltaY;
    s32 result = 0;

    *outX = targetX;
    *outY = targetY;
    if (index >= 0 && index < 4) {
        entry = &gOverlay59Entries[index];
        if (entry->owner != NULL) {
            deltaX = targetX - x;
            deltaY = targetY - y;
            if (useFirstScale) {
                deltaX *= entry->scale0;
                deltaY *= entry->scale0;
            } else {
                deltaX *= entry->scale4;
                deltaY *= entry->scale4;
            }
            *outX = (s32) deltaX + x;
            *outY = (s32) deltaY + y;
            result = 1;
        }
    }
    return result;
}
