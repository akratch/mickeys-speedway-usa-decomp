#include "PR/ultratypes.h"

/* Vertex-strip expansion; no exact DKR/JFG object match was found. */
typedef struct Overlay67Point {
    s8 useXOffset0;
    s8 x0;
    s8 useYOffset0;
    s8 y0;
    s8 useXOffset1;
    s8 x1;
    s8 useYOffset1;
    s8 y1;
    s8 alpha;
} Overlay67Point;

typedef struct Overlay67Vertex {
    s16 x0;
    s16 y0;
    s16 x1;
    s16 y1;
    u32 color;
} Overlay67Vertex;

extern Overlay67Point *gOverlay67PointSets[];
void overlay67DrawReloc(void *displayList, s32 count,
                        Overlay67Vertex *vertices, s32 flags);

void overlay67BuildVertices(void *displayList, u8 setIndex, s32 x, s32 y,
                            s32 xOffset, s32 yOffset, u32 color) {
    register Overlay67Point *point;
    register Overlay67Vertex *vertex;
    Overlay67Vertex vertices[24];
    register s32 count;
    register s32 alpha;

    point = gOverlay67PointSets[setIndex];
    alpha = color & 0xFF;
    vertex = vertices;
    count = 0;
    if ((color &= ~0xFF, point->useXOffset0 >= 0)) {
        do {
            vertex->x0 = point->x0 + x;
            if (point->useXOffset0 != 0) {
                vertex->x0 += xOffset;
            }
            vertex->y0 = point->y0 + y;
            if (point->useYOffset0 != 0) {
                vertex->y0 += yOffset;
            }
            vertex->x1 = point->x1 + x;
            if (point->useXOffset1 != 0) {
                vertex->x1 += xOffset;
            }
            vertex->y1 = point->y1 + y;
            if (point->useYOffset1 != 0) {
                vertex->y1 += yOffset;
            }
            count++;
            point++;
            vertex->color = color | ((point[-1].alpha * alpha) >> 7);
            vertex++;
            if (point->useXOffset0 < 0) {
                break;
            }
        } while (count != 24);
    }
    overlay67DrawReloc(displayList, count, vertices, 0);
}
