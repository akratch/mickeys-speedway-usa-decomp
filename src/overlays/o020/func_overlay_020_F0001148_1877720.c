#include "PR/ultratypes.h"

typedef struct Overlay20TailVector {
    f32 x;
    f32 y;
    f32 z;
} Overlay20TailVector;

typedef struct Overlay20TailVertex {
    s16 x;
    s16 value;
    s16 y;
    u8 red;
    u8 green;
    u8 blue;
    u8 pad9;
} Overlay20TailVertex;

typedef struct Overlay20TailGrid {
    s16 minX;
    s16 baseValue;
    s16 minY;
    s16 width;
    s16 height;
    s16 columns;
    s16 rows;
    u8 pad0E[4];
    u8 bufferIndex;
    u8 baseColor;
    f32 negativeScale;
    f32 positiveScale;
    f32 displacementScale;
    Overlay20TailVertex *buffers[2];
} Overlay20TailGrid;

extern f32 sqrtf(f32 value);
extern f32 gOverlay20TailXLimit;
extern f32 gOverlay20TailYLimit;

/* Plateau (2026-08-26, p5): workbench mixed, 207/205 instructions and 180 raw (178 masked) differing words from +0x20; both frames are 0x90.
 * The mips3/r4300-mul flag sweep and init/type/comparison/volatile/neighbor/lifetime/permuter probes leave 17 structural and 114 register rows.
 * No untried source-backed lever remains; retain NON_MATCHING. */
#ifdef NON_MATCHING
f32 func_overlay_020_F0001148_1877720(Overlay20TailGrid *grid, f32 x, f32 y,
    Overlay20TailVector *normal) {
    Overlay20TailVertex *vertex;
    Overlay20TailVertex *nextRow;
    f32 cellWidth;
    f32 cellHeight;
    f32 localX;
    f32 localY;
    f32 normalY;
    f32 result;
    f32 normalZ;
    f32 normalX;
    volatile f32 rowValue;
    f32 baseValue;
    f32 planeX;
    f32 cornerValue;
    f32 length;
    s32 column;
    s32 row;
    s32 stride;
    s32 upperTriangle;

    cellWidth = (f32)grid->width / (f32)grid->columns;
    cellHeight = (f32)grid->height / (f32)grid->rows;
    stride = grid->rows + 1;
    result = (f32)grid->baseValue;
    localX = x;
    localY = y;
    localX -= (f32)grid->minX;
    localY -= (f32)grid->minY;
    if (localX < 0.0f) {
        localX = 0.0f;
    } else if ((f32)grid->width <= localX) {
        localX = (f32)grid->width - gOverlay20TailXLimit;
    }
    if (localY < 0.0f) {
        localY = 0.0f;
    } else if ((f32)grid->height <= localY) {
        localY = (f32)grid->height - gOverlay20TailYLimit;
    }

    column = (s32)(localX / cellWidth);
    row = (s32)(localY / cellHeight);
    localX -= (f32)column * cellWidth;
    localY -= (f32)row * cellHeight;
    upperTriangle = 0;
    if ((localY != cellHeight) &&
        (localX < (((cellHeight - localY) / cellHeight) * cellWidth))) {
        upperTriangle = 1;
    }

    vertex = grid->buffers[grid->bufferIndex] + (column + (row * stride));
    if (upperTriangle != 0) {
        baseValue = (f32)vertex->value;
        nextRow = vertex + stride;
        rowValue = (f32)nextRow->value;
        planeX = 0.0f;
        normalX = (baseValue - (f32)vertex[1].value) * cellHeight;
        normalY = cellHeight * cellWidth;
        normalZ = (baseValue - rowValue) * cellWidth;
    } else {
        baseValue = (f32)vertex[1].value;
        nextRow = vertex + stride;
        rowValue = (f32)nextRow->value;
        planeX = cellWidth;
        cornerValue = (f32)nextRow[1].value;
        normalX = (rowValue - cornerValue) * cellHeight;
        normalY = cellHeight * cellWidth;
        normalZ = (baseValue - cornerValue) * cellWidth;
    }

    length = sqrtf((normalX * normalX) + (normalY * normalY) +
                   (normalZ * normalZ));
    if ((length != 0.0f) && (normalY != 0.0f)) {
        normalX /= length;
        normalZ /= length;
        normalY /= length;
        result = -(((normalX * localX) + (normalZ * localY)) -
                   ((planeX * normalX) + (baseValue * normalY))) /
                 normalY;
    }
    if (normal != NULL) {
        normal->x = normalX;
        normal->y = normalY;
        normal->z = normalZ;
    }
    return result;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o020/func_overlay_020_F0001148_1877720/func_overlay_020_F0001148_1877720.s")
#endif
