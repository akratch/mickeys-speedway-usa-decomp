#include "PR/ultratypes.h"
typedef struct O1Point2 { f32 x; f32 y; } O1Point2;
typedef struct O1Reference { u8 pad00[0xC]; f32 x; u8 pad10[4]; f32 y; } O1Reference;
typedef struct O1MotionWorld { u8 pad00[0x37C]; s16 angle; u8 pad37E[0x1A]; f32 scale; f32 heading; } O1MotionWorld;
extern O1Point2 *D_20C;
extern O1Point2 *D_210;
extern O1Reference *D_1D9C;
extern O1MotionWorld *D_1DA0;
extern f32 overlay1SquareRoot(f32 value);
extern s32 overlay1AngleFromIndex(s16 value);
void overlay1InitMotionScale(void) {
    f32 dx;
    f32 dy;
    f32 firstDistance;
    f32 secondDistance;
    dx = D_20C->x - D_210->x;
    dy = D_20C->y - D_210->y;
    firstDistance = overlay1SquareRoot((dx * dx) + (dy * dy));
    dx = D_20C->x - D_1D9C->x;
    dy = D_20C->y - D_1D9C->y;
    secondDistance = overlay1SquareRoot((dx * dx) + (dy * dy));
    D_1DA0->scale = secondDistance / firstDistance;
    D_1DA0->heading = (f32)overlay1AngleFromIndex(D_1DA0->angle) + D_1DA0->scale;
}
