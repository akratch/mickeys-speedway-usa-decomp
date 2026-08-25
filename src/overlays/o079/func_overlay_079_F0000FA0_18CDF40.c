#include "PR/ultratypes.h"

typedef struct Overlay79Vector {
    f32 x;
    f32 y;
    f32 z;
} Overlay79Vector;

typedef struct Overlay79Plane {
    Overlay79Vector normal;
    f32 constant;
    Overlay79Vector origin;
    f32 distance;
    u32 flags;
} Overlay79Plane;

typedef struct Overlay79CollisionState {
    u8 pad0[8];
    u32 flags;
} Overlay79CollisionState;

typedef struct Overlay79CollisionObject {
    u8 pad0[0x64];
    Overlay79CollisionState *state;
} Overlay79CollisionObject;

extern f32 sqrtf(f32 value);
extern s32 Arctanf(f32 y, f32 x);
extern f32 func_8002A8BC(s32 angle);
extern f32 gOverlay79CollisionMinimumY;
extern f32 gOverlay79CollisionEpsilon;
extern f32 gOverlay79CollisionLift;
extern f32 gOverlay79CollisionProjection;

/* Workbench: structure-mismatch; best is 184/184 words, 143 differences, first +0x38.
 * Tried exact-frame/spill census, volatile homes, declaration scopes, and expression association.
 * The 0x98 frame is exact; 29 structural and 95 register rows remain after the FP pool split. */
#ifdef NON_MATCHING
void func_overlay_079_F0000FA0_18CDF40(
    void *unused, Overlay79Vector *position, Overlay79Vector *axis,
    f32 distance, Overlay79Plane *plane, Overlay79CollisionObject *object) {
    Overlay79CollisionState *state;
    register f32 ny;
    f32 nx;
    f32 nz;
    f32 crossZ;
    f32 projectedX;
    f32 projectedY;
    f32 projectedZ;
    f32 crossX;
    f32 crossY;
    f32 value;
    f32 length;
    f32 amount;
    volatile f32 planeConstant;
    volatile f64 framePad;

    (void)unused;
    ny = plane->normal.y;
    nx = plane->normal.x;
    nz = plane->normal.z;
    state = object->state;
    planeConstant = plane->constant;
    if ((gOverlay79CollisionMinimumY <= ny) ||
        ((plane->flags & 0x10000000) != 0)) {
        f32 length;

        crossX = axis->z * ny;
        crossY = (nz * axis->x) - (axis->z * nx);
        crossZ = -(axis->x * ny);
        projectedX = (crossY * nz) - (crossZ * ny);
        projectedY = (crossZ * nx) - (crossX * nz);
        projectedZ = (crossX * ny) - (crossY * nx);
        value = (projectedX * projectedX) +
                (projectedY * projectedY) +
                (projectedZ * projectedZ);
        if (gOverlay79CollisionEpsilon < value) {
            length = sqrtf(value);
            projectedY /= length;
            projectedX /= length;
            projectedZ /= length;
            amount = distance - plane->distance;
            position->x = plane->origin.x + (amount * projectedX);
            position->y = plane->origin.y + (amount * projectedY);
            position->z = plane->origin.z + (amount * projectedZ);
        } else {
            position->y = (-((position->z * nz) +
                             (nx * position->x) + planeConstant) /
                           ny) +
                          gOverlay79CollisionLift;
        }
        state->flags |= 2;
        return;
    }

    {
        f32 length;

    crossX = position->x;
    crossY = position->y;
    crossZ = position->z;
    amount = gOverlay79CollisionProjection -
            ((crossZ * nz) + ((nx * crossX) + (ny * crossY)) +
             planeConstant);
    projectedX = crossX + (amount * nx);
    projectedY = crossY + (amount * ny);
    projectedZ = crossZ + (amount * nz);
    crossX -= projectedX;
    crossZ -= projectedZ;
    crossY -= projectedY;
    length = sqrtf((crossX * crossX) + (crossZ * crossZ));
    value = func_8002A8BC((s16)Arctanf(crossY, length));
    if (value != 0.0f) {
        amount /= value;
        length = sqrtf((nx * nx) + (nz * nz));
        position->x += amount * (nx / length);
        position->z += amount * (nz / length);
    } else {
        position->x = projectedX;
        position->z = projectedZ;
        position->y = projectedY;
    }
    state->flags |= 4;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o079/func_overlay_079_F0000FA0_18CDF40/func_overlay_079_F0000FA0_18CDF40.s")
#endif
