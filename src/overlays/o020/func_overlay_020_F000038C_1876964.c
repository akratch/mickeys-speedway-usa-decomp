#include "PR/ultratypes.h"

typedef struct Overlay20InitVertex {
    s16 x;
    s16 value;
    s16 y;
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
} Overlay20InitVertex;

typedef struct Overlay20InitTile {
    u8 command0;
    u8 index0;
    u8 index1;
    u8 index9;
    s16 x0a;
    s16 y0a;
    s16 x1a;
    s16 y0b;
    s16 x0b;
    s16 y1a;
    u8 command1;
    u8 index1b;
    u8 index9b;
    u8 index10;
    s16 x1b;
    s16 y0c;
    s16 x0c;
    s16 y1b;
    s16 x1c;
    s16 y1c;
} Overlay20InitTile;

typedef struct Overlay20InitTexture {
    u8 pad00[6];
    u16 width;
    u16 height;
} Overlay20InitTexture;

typedef struct Overlay20InitGrid {
    s16 minX;
    s16 baseValue;
    s16 minY;
    s16 width;
    s16 height;
    s16 columns;
    s16 rows;
    s16 textureScaleX;
    s16 textureScaleY;
    u8 bufferIndex;
    u8 baseColor;
    f32 negativeScale;
    f32 positiveScale;
    f32 displacementScale;
    Overlay20InitVertex *buffers[2];
    Overlay20InitTile *tiles;
    Overlay20InitTexture *texture;
} Overlay20InitGrid;

extern void *func_overlay_020_F0000000_18765D8();

/* PLATEAU (2026-08-26): workbench structure-mismatch; best 256/270 words, first +0x0.
 * Flag lattice, top-level texture lifetime, cursor, command/store, and saved-register probes did not close it.
 * Candidate remains 6 instructions short with a 0x38 frame versus target 0x40. */
#ifdef NON_MATCHING
Overlay20InitGrid *func_overlay_020_F000038C_1876964(Overlay20InitGrid *grid) {
    Overlay20InitVertex *vertex;
    Overlay20InitTile *tile;
    s32 bufferIndex;
    s32 vertexBytes;
    s32 tileBytes;
    s32 column;
    s32 row;
    s32 columnNext;
    s32 rowNext;
    s32 textureColumn;
    s32 x0;
    s32 x1;
    s32 y0;
    s32 y1;

    vertexBytes = (grid->columns + 1) * (grid->rows + 1) *
                  sizeof(Overlay20InitVertex);
    for (bufferIndex = 0; bufferIndex < 2; bufferIndex++) {
        vertex = grid->buffers[bufferIndex];
        if (vertex == NULL) {
            vertex = func_overlay_020_F0000000_18765D8(vertexBytes, 0x87);
            grid->buffers[bufferIndex] = vertex;
        }
        if (vertex != NULL) {
            for (row = 0; row <= grid->rows; row++) {
                for (column = 0; column <= grid->columns; column++) {
                    vertex->x = grid->minX +
                                ((column * grid->width) / grid->columns);
                    vertex->value = grid->baseValue;
                    vertex->y = grid->minY +
                                ((row * grid->height) / grid->rows);
                    vertex->red = 0xFF;
                    vertex->green = 0xFF;
                    vertex->blue = 0xFF;
                    vertex->alpha = 0xFF;
                    vertex++;
                }
            }
        }
    }

    tileBytes = (grid->columns * grid->rows) << 5;
    tile = grid->tiles;
    if (tile == NULL) {
        tile = func_overlay_020_F0000000_18765D8(tileBytes, 0x87);
        grid->tiles = tile;
    }
    if (tile != NULL) {
        for (row = 0; row < grid->rows; row++) {
            for (column = 0; column < grid->columns; column++) {
                columnNext = column + 1;
                rowNext = row + 1;
                textureColumn = column & 7;

                x0 = (((grid->textureScaleX * column) *
                       grid->texture->width) << 5) /
                     grid->columns;
                x1 = (((grid->textureScaleX * columnNext) *
                       grid->texture->width) << 5) /
                     grid->columns;
                y0 = (((grid->textureScaleY * row) *
                       grid->texture->height) << 5) /
                     grid->rows;
                y1 = (((grid->textureScaleY * rowNext) *
                       grid->texture->height) << 5) /
                     grid->rows;

                tile->command0 = 0x40;
                tile->index0 = textureColumn;
                tile->index1 = textureColumn + 1;
                tile->index9 = textureColumn + 9;
                tile->x0a = x0;
                tile->y0a = y0;
                tile->x1a = x1;
                tile->y0b = y0;
                tile->x0b = x0;
                tile->y1a = y1;
                tile->command1 = 0x40;
                tile->index1b = textureColumn + 1;
                tile->index9b = textureColumn + 9;
                tile->index10 = textureColumn + 10;
                tile->x1b = x1;
                tile->y0c = y0;
                tile->x0c = x0;
                tile->y1b = y1;
                tile->x1c = x1;
                tile->y1c = y1;
                tile++;
            }
        }
    }

    grid->bufferIndex = 1;
    if ((grid->buffers[0] == NULL) || (grid->buffers[1] == NULL) ||
        (grid->tiles == NULL)) {
        func_overlay_020_F0000000_18765D8(grid);
        grid = NULL;
    }
    return grid;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o020/func_overlay_020_F000038C_1876964/func_overlay_020_F000038C_1876964.s")
#endif
