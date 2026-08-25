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

/*
 * Plateau (2026-08-25, 10-attempt cap): the best -O2 -mips2
 * -Wab,-r4300_mul candidate has the exact 184-word size, differs in 150
 * words, and first diverges at +0x0.  The candidate uses a 0x90-byte frame
 * instead of the retail 0x98-byte frame; the remaining delta is dominated
 * by floating-point register allocation and spill lifetimes.  The full flag
 * lattice, the m2c-shaped formulation, and the structurally related Mickey
 * overlay 22/29 formulations all retain that allocation split.
 */
#ifdef NON_MATCHING
void func_overlay_079_F0000FA0_18CDF40(
    void *unused, Overlay79Vector *position, Overlay79Vector *axis,
    f32 distance, Overlay79Plane *plane, Overlay79CollisionObject *object) {
    Overlay79CollisionState *state;
    f32 nx;
    f32 ny;
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
    f32 planeConstant;

    (void)unused;
    ny = plane->normal.y;
    nx = plane->normal.x;
    nz = plane->normal.z;
    state = object->state;
    planeConstant = plane->constant;
    if ((gOverlay79CollisionMinimumY <= ny) ||
        ((plane->flags & 0x10000000) != 0)) {
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
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o079/func_overlay_079_F0000FA0_18CDF40/func_overlay_079_F0000FA0_18CDF40.s")
#endif
