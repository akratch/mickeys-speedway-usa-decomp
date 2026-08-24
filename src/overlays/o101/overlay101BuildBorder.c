#include "PR/ultratypes.h"

typedef struct Overlay101Gfx {
    u32 w0;
    u32 w1;
} Overlay101Gfx;

typedef struct Overlay101BorderRect {
    s16 x0;
    s16 y0;
    s16 x1;
    s16 y1;
    u32 color;
} Overlay101BorderRect;

void overlay101BuildIntensityColorsReloc(s32 intensity, s32 alpha, u32 *full,
                                         u32 *dim, u32 *dimmer, u32 *darkest);
void overlay101BuilderCreateReloc(Overlay101Gfx **displayList, s32 count,
                                  Overlay101BorderRect *rects, s32 flags);

/*
 * Overlay 101 text +0x2DC0..+0x2EFC.  Volatile color locals and an explicit
 * final-record pointer retain the exact private-frame compiler basin.
 */
void overlay101BuildBorder(Overlay101Gfx **displayList, s32 x, s32 y,
                           s32 width, s32 height, s32 intensity, s32 alpha,
                           s32 swapColors) {
    volatile u32 trailingColor;
    volatile u32 leadingColor;
    volatile u32 interiorColor;
    Overlay101BorderRect *rect;
    Overlay101BorderRect rects[5];

    if (swapColors != 0) {
        overlay101BuildIntensityColorsReloc(
            intensity + 1, alpha, (u32 *)&leadingColor,
            (u32 *)&trailingColor, (u32 *)&interiorColor, NULL);
    } else {
        overlay101BuildIntensityColorsReloc(
            intensity + 1, alpha, (u32 *)&trailingColor,
            (u32 *)&leadingColor, (u32 *)&interiorColor, NULL);
    }

    rect = rects;
    rect[0].x0 = x;
    rect[0].y0 = y;
    rect[0].x1 = x + 1;
    rect[0].y1 = y + height - 1;
    rect[0].color = leadingColor;

    rect[1].x0 = x;
    rect[1].y0 = y + height - 1;
    rect[1].x1 = x + width;
    rect[1].y1 = y + height;
    rect[1].color = leadingColor;

    rect[2].x0 = x + width - 1;
    rect[2].y0 = y + 1;
    rect[2].x1 = x + width;
    rect[2].y1 = y + height - 1;
    rect[2].color = trailingColor;

    rect[3].x0 = x + 1;
    rect[3].y0 = y;
    rect[3].x1 = x + width;
    rect[3].y1 = y + 1;
    rect[3].color = trailingColor;

    rect += 4;
    rect->x0 = x + 1;
    rect->y0 = y + 1;
    rect->x1 = x + width - 1;
    rect->y1 = y + height - 1;
    rect->color = interiorColor;

    overlay101BuilderCreateReloc(displayList, 5, rects, 0);
}
