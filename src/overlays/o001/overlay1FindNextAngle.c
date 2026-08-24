#include "PR/ultratypes.h"

typedef struct Overlay1AngleData {
    u8 pad00[0x39C];
    f32 angle;
} Overlay1AngleData;

typedef struct Overlay1AngleObject {
    u8 pad00[0x64];
    Overlay1AngleData *data;
} Overlay1AngleObject;

extern Overlay1AngleObject **overlay1GetAngleObjectsReloc(s32 *count);
extern f32 overlay1WrapOffset(f32 first, f32 second);
extern f32 gOverlay1NextAngleLimit;

/* DKR v77/v80 and JFG contain no exact donor for this angle-selection scan. */
#ifdef NON_MATCHING
Overlay1AngleObject *overlay1FindNextAngle(f32 angle) {
    s32 count;
    Overlay1AngleObject **objects;
    Overlay1AngleObject **cursor;
    Overlay1AngleObject *object;
    Overlay1AngleObject *best;
    f32 difference;
    f32 bestDifference;
    s32 remaining;
    objects = overlay1GetAngleObjectsReloc(&count);
    bestDifference = gOverlay1NextAngleLimit;
    best = (Overlay1AngleObject *)(count - count);
    if (count != 0) {
        remaining = count - 1;
        cursor = (Overlay1AngleObject **)((u8 *)objects + (remaining << 2));
        do {
            object = *cursor;
            objects = (Overlay1AngleObject **)object->data;
            difference = overlay1WrapOffset(
                angle, ((Overlay1AngleData *)objects)->angle);
            if ((difference > 0.0f) && (difference < bestDifference)) {
                bestDifference = difference;
                best = object;
            }
            cursor = (Overlay1AngleObject **)((u8 *)cursor - 4);
        } while (remaining--);
    }
    return best;
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay1FindNextAngle/func_overlay_001_F0002744_184EB24.s")
#endif
