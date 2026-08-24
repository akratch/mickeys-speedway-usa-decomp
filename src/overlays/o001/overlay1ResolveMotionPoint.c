#include "PR/ultratypes.h"
typedef struct O1PathOwner { s16 angle; u8 pad02[0xA]; f32 x; f32 y; f32 z; } O1PathOwner;
extern s32 D_0;
extern f32 D_B4;
extern f32 D_B8;
extern s32 overlay1HasPathData(void);
extern void overlay1InterpolatePath(f32 *x, f32 *z, s32 path, f32 offset);
extern f32 overlay1SinAngle(s16 angle);
extern f32 overlay1CosAngle(s16 angle);
extern f32 overlay1SquareRoot(f32 value);
void overlay1ResolveMotionPoint(O1PathOwner *owner, s32 path, f32 *outX,
                                f32 *outY, f32 *outZ) {
    f32 dx;
    f32 dz;
    f32 distance;
    f32 scale;
    if (overlay1HasPathData() == 0) {
        *outX = 0.0f;
        *outY = 0.0f;
        *outZ = 0.0f;
        return;
    }
    if (D_0 == 1) {
        overlay1InterpolatePath(outX, outZ, path, 1.0f);
        dx = *outX - owner->x;
        dz = *outZ - owner->z;
        distance = overlay1SquareRoot((dx * dx) + (dz * dz));
        if (distance > 0.0f) {
            scale = 1.0f / distance;
            dx *= scale;
            dz *= scale;
        }
        *outX = owner->x + (dx * 150.0f);
        *outY = owner->y + D_B4;
        *outZ = owner->z + (dz * 150.0f);
    } else {
        *outX = owner->x + (overlay1SinAngle(owner->angle) * 150.0f);
        *outY = owner->y + D_B8;
        *outZ = owner->z + (overlay1CosAngle(owner->angle) * 150.0f);
    }
}
