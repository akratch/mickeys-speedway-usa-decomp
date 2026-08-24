#include "PR/ultratypes.h"

typedef struct Overlay1PreviousAngleData {
    u8 pad00[0x39C];
    f32 angle;
} Overlay1PreviousAngleData;

typedef struct Overlay1PreviousAngleObject {
    u8 pad00[0x64];
    Overlay1PreviousAngleData *data;
} Overlay1PreviousAngleObject;

extern Overlay1PreviousAngleObject **overlay1GetAngleObjectsReloc(s32 *count);
extern f32 overlay1WrapOffset(f32 first, f32 second);
extern f32 gOverlay1PreviousAngleLimit;

/* DKR v77/v80 and JFG contain no exact donor for this angle-selection scan. */
Overlay1PreviousAngleObject *overlay1FindPreviousAngle(f32 angle) {
    s32 count;
    Overlay1PreviousAngleObject **objects;
    Overlay1PreviousAngleObject **cursor;
    Overlay1PreviousAngleObject *object;
    Overlay1PreviousAngleObject *best;
    f32 difference;
    f32 bestDifference;
    s32 remaining;

    objects = overlay1GetAngleObjectsReloc(&count);
    bestDifference = gOverlay1PreviousAngleLimit;
    best = (Overlay1PreviousAngleObject *)(count - count);
    if (count != 0) {
        remaining = count - 1;
        cursor = (Overlay1PreviousAngleObject **)((u8 *)objects +
                                                  (remaining << 2));
        do {
            object = *cursor;
            objects = (Overlay1PreviousAngleObject **)object->data;
            difference = overlay1WrapOffset(
                ((Overlay1PreviousAngleData *)objects)->angle, angle);
            if ((difference > 0.0f) && (difference < bestDifference)) {
                bestDifference = difference;
                best = object;
            }
            cursor = (Overlay1PreviousAngleObject **)((u8 *)cursor - 4);
        } while (remaining--);
    }
    return best;
}
