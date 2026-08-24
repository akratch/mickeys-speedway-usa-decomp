#include "PR/ultratypes.h"

typedef struct O1ControlPoint { f32 x; f32 z; u8 pad08[8]; } O1ControlPoint;
typedef struct O1ControlTable { u8 pad00[0x14]; O1ControlPoint points[1]; } O1ControlTable;
typedef struct O1PathOwner { u8 pad00[0x398]; f32 pathOffset; } O1PathOwner;

extern O1PathOwner *D_1DA0;
extern O1ControlTable *D_1D60;
extern O1ControlTable *D_1D64;
extern O1ControlTable *D_1D68;
extern O1ControlTable *D_1D6C;
extern O1ControlTable *overlay1NextControlTable(O1ControlTable *table);
extern f32 overlay1CubicInterpolate(f32 a, f32 b, f32 c, f32 d, f32 t);

#ifdef NON_MATCHING
void overlay1InterpolatePath(f32 *outX, f32 *outZ, s32 path, f32 offset) {
    O1ControlTable *table3Base;
    O1ControlPoint *point0;
    O1ControlPoint *point1;
    O1ControlPoint *point2;
    O1ControlPoint *point3;
    f32 position;
    f32 fraction;
    s32 whole;
    s32 originalWhole;
    s32 remaining;

    position = D_1DA0->pathOffset + offset;
    point0 = &D_1D60->points[path];
    point1 = &D_1D64->points[path];
    point2 = &D_1D68->points[path];
    table3Base = D_1D6C;
    point3 = &table3Base->points[path];
    whole = (s32) position;
    originalWhole = whole;
    remaining = whole - 1;

    if (whole != 0) {
        do {
            table3Base = overlay1NextControlTable(table3Base);
            point0 = point1;
            point1 = point2;
            point2 = point3;
            point3 = &table3Base->points[path];
            whole = remaining;
            remaining--;
        } while (whole != 0);
    }

    fraction = position - (f32) originalWhole;
    *outX = overlay1CubicInterpolate(point0->x, point1->x, point2->x,
                                     point3->x, fraction);
    *outZ = overlay1CubicInterpolate(point0->z, point1->z, point2->z,
                                     point3->z, fraction);
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay1InterpolatePath/func_overlay_001_F0000CA8_184D088.s")
#endif
