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

/* Workbench: allocation mismatch; exact 131-word size/0x78 frame, with three non-relocation differences, first +0x78.
 * Levers: projection normalization, scalar-home ordering, result/constant scheduling, and square-root-result reuse.
 * Remaining: one commutative operand order and one call-preservation home; compound and separate-result forms regressed. */
#ifdef NON_MATCHING
void func_overlay_026_F0000B18_187AF10(
    s32 unused, O26ProjectionVec3f *out, O26ProjectionVec3f *direction,
    f32 distance, O26ProjectionPlane *plane, O26ProjectionOwner *owner) {
    O26ProjectionResult *result;
    f32 crossX;
    f32 crossY;
    f32 crossZ;
    f32 planeConstant;
    f32 projectedX;
    f32 projectedY;
    f32 projectedZ;
    f32 lengthSquared;
    f32 amount;
    f32 normalY = plane->normal.y;
    f32 normalX = plane->normal.x;
    f32 normalZ = plane->normal.z;

    result = owner->result64;
    planeConstant = plane->constant;
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
            lengthSquared = sqrtf(lengthSquared);
            projectedX /= lengthSquared;
            projectedY /= lengthSquared;
            projectedZ /= lengthSquared;
            amount = distance - plane->distance;
            out->x = plane->point.x + (amount * projectedX);
            out->y = plane->point.y + (amount * projectedY);
            out->z = plane->point.z + (amount * projectedZ);
        } else {
            out->y = (-((out->z * normalZ) +
                        (normalX * out->x) + planeConstant) /
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
