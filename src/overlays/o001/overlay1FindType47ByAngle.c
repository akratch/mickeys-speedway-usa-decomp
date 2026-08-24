#include "PR/ultratypes.h"

typedef struct Overlay1ScanData {
    u8 pad00[8];
    u16 phase;
} Overlay1ScanData;

typedef struct Overlay1ScanObject {
    u8 pad00[0x44];
    s16 type;
    u8 pad46[0x1E];
    Overlay1ScanData *data;
} Overlay1ScanObject;

extern Overlay1ScanObject **overlay1GetAngleObjectsReloc(
    s32 *start, s32 *end);
extern f32 overlay1WrapOffset(f32 first, f32 second);
extern f32 gOverlay1ScanLimit;
extern f32 gOverlay1PhaseScale;

#ifdef NON_MATCHING
Overlay1ScanObject *overlay1FindType47ByAngle(f32 angle) {
    s32 start;
    s32 end;
    Overlay1ScanObject **objects;
    Overlay1ScanObject **cursor;
    Overlay1ScanObject *object;
    Overlay1ScanObject *best;
    f32 difference;
    f32 bestDifference;
    f32 scale;
    s32 index;

    objects = overlay1GetAngleObjectsReloc(&start, &end);
    bestDifference = gOverlay1ScanLimit;
    best = NULL;
    index = start;
    if (start < end) {
        scale = gOverlay1PhaseScale;
        cursor = objects + start;
        do {
            object = *cursor;
            if (object->type == 0x2F) {
                difference = overlay1WrapOffset(
                    (f32)object->data->phase * scale, angle);
                if ((difference > 0.0f) && (difference < bestDifference)) {
                    bestDifference = difference;
                    best = object;
                }
            }
            index++;
            cursor++;
        } while (index < end);
    }
    return best;
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay1FindType47ByAngle/func_overlay_001_F00001AC_184C58C.s")
#endif
