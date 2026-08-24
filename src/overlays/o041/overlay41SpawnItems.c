#include "PR/ultratypes.h"

typedef struct Overlay41Root {
    u8 pad0[8];
    void *item;
} Overlay41Root;

typedef struct Overlay41Item {
    u8 pad0[0xC];
    f32 x;
    f32 y;
    s32 kind;
    u8 pad18[0x38];
    f32 *opacity;
} Overlay41Item;

typedef struct Overlay41ShortPair {
    s16 x;
    s16 y;
    s16 z;
} Overlay41ShortPair;

typedef struct Overlay41FloatVector {
    f32 x;
    f32 y;
    f32 z;
} Overlay41FloatVector;

/* Fresh pinned DKR v77/v80 and JFG scans found no exact donor. */
extern Overlay41Root **gOverlay41Roots;
extern s32 overlay41RandomRange(s32 lower, s32 upper);
extern void overlay41ConvertPair(Overlay41ShortPair *input,
                                 Overlay41FloatVector *output);
extern void overlay41Emit(f32 x, f32 y, s32 kind, f32 outX, f32 outY,
                          f32 z, s32 opacity, s32 mode, f32 scale);

#ifdef NON_MATCHING
void overlay41SpawnItems(s32 rootIndex, s32 count, s32 mode, s32 centerX,
                         s32 centerY, s32 radiusX, s32 radiusY, s32 centerZ,
                         s32 radiusZ) {
    s32 opacity;
    Overlay41ShortPair input;
    Overlay41FloatVector output;
    Overlay41Root *root;
    Overlay41Item *item;
    f32 scale;

    root = gOverlay41Roots[rootIndex];
    scale = 1.0f;
    if (root == 0) {
        return;
    }
    item = root->item;
    if (item == 0) {
        return;
    }

    switch (mode) {
        case 0:
            mode = 0;
            break;
        case 1:
            mode = 1;
            break;
        case 2:
            mode = 1;
            scale = 0.5f;
            break;
        case 3:
            mode = 5;
            scale = 0.75f;
            break;
        case 4:
            mode = 4;
            break;
        default:
            mode = 2;
            break;
    }

    if (item->opacity != 0) {
        opacity = *item->opacity * 255.0f;
    } else {
        opacity = 255;
    }

    while (count--) {
        input.x = overlay41RandomRange(-radiusX, radiusX) + centerX;
        input.y = overlay41RandomRange(-radiusY, radiusY) + centerY;
        output.z =
            (f32)(overlay41RandomRange(-radiusZ, radiusZ) + centerZ) * -0.01f;
        overlay41ConvertPair(&input, &output);
        overlay41Emit(item->x, item->y, item->kind, output.x, output.y, output.z,
                      opacity, mode, scale);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o041/overlay41SpawnItems/func_overlay_041_F0001740_1888A78.s")
#endif
