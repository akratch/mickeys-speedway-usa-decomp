#include "PR/ultratypes.h"

typedef struct Overlay1Direction {
    f32 x;
    u8 pad4[4];
    f32 z;
    s16 angle;
} Overlay1Direction;

extern s32 overlay1DirectionReloc(f32, f32);
extern s32 overlay1CompareDirectionReloc(s32, s16);

/* Exact at +0x758; DKR v77/v80 and JFG have no exact donor for this predicate. */
s32 overlay1TestDirection(Overlay1Direction *direction, f32 x, f32 z) {
    s32 angle;

    angle = overlay1DirectionReloc(x - direction->x, z - direction->z);
    return overlay1CompareDirectionReloc(angle, direction->angle) > 0;
}
