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

extern f32 D_10;
extern f32 func_overlay_022_F0000000_1878108(f32);

/* Workbench: mixed constant/commutative; best differs 7/173 words, first linked +0x74.
 * Compound assignment, spill-padding widths, and a projected-vector aggregate were tried.
 * Three mul operand encodings and X/Z homes +0x50/+0x48 vs +0x54/+0x4C remain. */
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

    if ((D_10 <= ny) || (((s32)plane->flags << 3) < 0)) {
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
            lengthSquared = func_overlay_022_F0000000_1878108(lengthSquared);
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
            f32 scale;

            dot = (direction->z * nz) +
                  ((nx * direction->x) + (ny * direction->y));
            scale = -dot;
            scale += scale;
            projectedX = direction->x + (scale * nx);
            projectedY = direction->y + (scale * ny);
            projectedZ = direction->z + (scale * nz);
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
