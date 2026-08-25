#include "PR/ultratypes.h"

/* The target's FP multiply schedule requires -Wab,-r4300_mul. */

typedef struct O26ProjectionVec3f {
    f32 x;
    f32 y;
    f32 z;
} O26ProjectionVec3f;

typedef struct O26ProjectionPlane {
    O26ProjectionVec3f normal;
    f32 constant;
    O26ProjectionVec3f point;
    f32 distance;
    u32 flags;
} O26ProjectionPlane;

typedef struct O26ProjectionResult {
    u8 pad00[8];
    O26ProjectionVec3f normal08;
    O26ProjectionVec3f normal14;
    u8 pad20[8];
    u32 flags28;
} O26ProjectionResult;

typedef struct O26ProjectionOwner {
    u8 pad00[0x64];
    O26ProjectionResult *result64;
} O26ProjectionOwner;

extern f32 D_14;
extern f32 D_18;
extern f32 D_1C;
extern f32 sqrtf(f32 value);

/*
 * NON_MATCHING: best coherent candidate is 129/131 instructions with its
 * first mismatch at +0x0: IDO uses a 0x70-byte frame instead of the target's
 * 0x78-byte frame.  The target keeps five FP values in callee-saved registers
 * across sqrtf, while this source keeps two and spills the remaining values.
 * The flag lattice and a bounded permuter run did not recover that lifetime
 * shape; the permuter's lower-scoring candidate used uninitialized values.
 */
#ifdef NON_MATCHING
void func_overlay_026_F0000B18_187AF10(
    s32 unused, O26ProjectionVec3f *out, O26ProjectionVec3f *direction,
    f32 distance, O26ProjectionPlane *plane, O26ProjectionOwner *owner) {
    O26ProjectionResult *result;
    register f32 projectedY;
    register f32 projectedZ;
    register f32 normalY;
    register f32 normalX;
    register f32 normalZ;
    f32 crossX;
    f32 crossY;
    f32 crossZ;
    f32 projectedX;
    f32 lengthSquared;
    f32 length;
    f32 amount;

    normalY = plane->normal.y;
    normalX = plane->normal.x;
    normalZ = plane->normal.z;
    result = owner->result64;
    if ((D_14 <= normalY) || (plane->flags & 0x10000000)) {
        crossX = direction->z * normalY;
        crossY = (normalZ * direction->x) -
                 (direction->z * normalX);
        crossZ = -(direction->x * normalY);

        projectedX = (crossY * normalZ) - (crossZ * normalY);
        projectedY = (crossZ * normalX) - (crossX * normalZ);
        projectedZ = (crossX * normalY) - (crossY * normalX);
        lengthSquared = (projectedX * projectedX) +
                        (projectedY * projectedY) +
                        (projectedZ * projectedZ);

        if (D_18 < lengthSquared) {
            length = sqrtf(lengthSquared);
            amount = distance - plane->distance;
            out->x = plane->point.x + (amount * (projectedX / length));
            out->y = plane->point.y + (amount * (projectedY / length));
            out->z = plane->point.z + (amount * (projectedZ / length));
        } else {
            out->y = (-((out->z * normalZ) +
                        (normalX * out->x) + plane->constant) /
                      normalY) +
                     D_1C;
        }

        result->normal08.x = normalX;
        result->normal08.y = normalY;
        result->normal08.z = normalZ;
        result->flags28 |= 2;
    } else {
        out->x = plane->point.x;
        out->y = plane->point.y;
        out->z = plane->point.z;
        result->normal14.x = normalX;
        result->normal14.y = normalY;
        result->normal14.z = normalZ;
        result->flags28 |= 4;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o026/func_overlay_026_F0000B18_187AF10/func_overlay_026_F0000B18_187AF10.s")
#endif
