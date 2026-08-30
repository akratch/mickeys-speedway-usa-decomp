#include "PR/ultratypes.h"

typedef struct Vec3f {
    f32 x;
    f32 y;
    f32 z;
} Vec3f;

typedef struct Overlay22Plane {
    Vec3f normal;
    u8 pad0C[4];
    Vec3f point;
    f32 distance;
    u32 flags;
} Overlay22Plane;

typedef struct Overlay22Result {
    u8 pad00;
    u8 flags01;
    s16 mode02;
    u8 pad04[4];
    Vec3f normal08;
    Vec3f source14;
    Vec3f point20;
} Overlay22Result;

typedef struct Overlay22Owner {
    u8 pad00[0x64];
    Overlay22Result *result64;
} Overlay22Owner;

extern f32 D_0[];
extern f32 sqrtf(f32);

/* Workbench plateau: allocation mismatch. Reusing crossZ across exclusive
 * branches fixed both call-stack homes while preserving the 0x88-byte frame.
 * Three commutative multiply operand-order encodings remain; compound forms
 * escape function ownership, so GLOBAL_ASM remains canonical. */
#ifdef NON_MATCHING
void func_overlay_022_F0000A7C_1878B84(
    void *unused, Vec3f *out, Vec3f *direction, f32 distance,
    Overlay22Plane *plane, Overlay22Owner *owner) {
    Overlay22Result *result;
    f32 nx;
    f32 ny;
    f32 nz;
    f32 crossZ;
    f32 projectedX;
    f32 projectedY;
    f32 projectedZ;
    f32 crossX;
    f32 crossY;
    f32 lengthSquared;

    result = owner->result64;
    nx = plane->normal.x;
    ny = plane->normal.y;
    nz = plane->normal.z;

    if ((D_0[4] <= ny) || (((s32)plane->flags << 3) < 0)) {
        crossX = (ny * direction->z) - (direction->y * nz);
        crossY = (nz * direction->x) - (direction->z * nx);
        crossZ = (nx * direction->y) - (direction->x * ny);

        projectedX = (crossY * nz) - (crossZ * ny);
        projectedY = (crossZ * nx) - (crossX * nz);
        projectedZ = (crossX * ny) - (crossY * nx);
        lengthSquared = (projectedX * projectedX) +
                        (projectedY * projectedY) +
                        (projectedZ * projectedZ);

        if (0.0f < lengthSquared) {
            lengthSquared = sqrtf(lengthSquared);
            projectedY /= lengthSquared;
            projectedX /= lengthSquared;
            projectedZ /= lengthSquared;
            lengthSquared = distance - plane->distance;
            out->x = plane->point.x + (lengthSquared * projectedX);
            out->y = plane->point.y + (lengthSquared * projectedY);
            out->z = plane->point.z + (lengthSquared * projectedZ);
        } else {
            out->x = plane->point.x;
            out->y = plane->point.y;
            out->z = plane->point.z;
        }

        result->normal08.x = nx;
        result->normal08.y = ny;
        result->flags01 |= 2;
        result->normal08.z = nz;
    } else {
        if (result->mode02 != 0) {
            out->x = plane->point.x;
            out->y = plane->point.y;
            out->z = plane->point.z;
        } else {
            f32 dot;

            dot = (direction->z * nz) +
                  ((nx * direction->x) + (ny * direction->y));
            crossZ = -dot;
            crossZ += crossZ;
            projectedX = direction->x + (crossZ * nx);
            projectedY = direction->y + (crossZ * ny);
            projectedZ = direction->z + (crossZ * nz);
            lengthSquared = distance - plane->distance;
            out->x = plane->point.x + (lengthSquared * projectedX);
            out->y = plane->point.y + (lengthSquared * projectedY);
            out->z = plane->point.z + (lengthSquared * projectedZ);

            result->point20.x = plane->point.x;
            result->point20.y = plane->point.y;
            result->point20.z = plane->point.z;
            result->source14.x = nx;
            result->source14.y = ny;
            result->source14.z = nz;
        }
        result->flags01 |= 4;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o022/overlay22ResolvePlane/func_overlay_022_F0000A7C_1878B84.s")
#endif

/* PLATEAU-HANDOFF:func_overlay_022_F0000A7C_1878B84:start
 * symbol: func_overlay_022_F0000A7C_1878B84
 * score: 170/173 words
 * frame: 0x88
 * relocations: 3
 * first-mismatch: +0x74
 * summary: Three commutative operand-order encodings remain; lifetime reuse fixed both call-stack homes; all 3/3 relocation identities authenticate.
 * PLATEAU-HANDOFF:func_overlay_022_F0000A7C_1878B84:end
 */
