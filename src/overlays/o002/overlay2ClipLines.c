#include "PR/ultratypes.h"

typedef struct Overlay2Line {
    f32 x1;
    f32 y1;
    f32 x2;
    f32 y2;
    u16 value1;
    u16 value2;
} Overlay2Line;

typedef struct Overlay2LineRange {
    u8 pad0[0x10];
    u16 start;
    u16 count;
} Overlay2LineRange;

extern Overlay2Line *gOverlay2Lines;
extern s32 gOverlay2LineCount;
extern s32 gOverlay2BoundaryAxis;

extern void overlay2AppendLine(f32 x1, f32 y1, f32 x2, f32 y2, u16 value1,
                               u16 value2);
extern s32 overlay2ClassifyBoundary(f32 x1, f32 y1, f32 x2, f32 y2,
                                    s32 *side1, s32 *side2);
extern void overlay2IntersectBoundary(f32 x0, f32 y0, f32 x1, f32 y1,
                                      f32 *outX, f32 *outY);

/* Pinned DKR v77/v80 and JFG scans found no matching clipping body. */
s32 overlay2ClipLines(Overlay2LineRange *input, Overlay2LineRange *output,
                      s32 wantedSide) {
    Overlay2Line *line;
    s32 remaining;
    s32 side1;
    s32 side2;
    s32 xSide;
    s32 ySide;
    f32 intersectionX;
    f32 intersectionY;

    remaining = input->count;
    line = &gOverlay2Lines[input->start];
    output->start = gOverlay2LineCount;
    while (remaining--) {
        if (overlay2ClassifyBoundary(line->x1, line->y1, line->x2, line->y2,
                                     &side1, &side2)) {
            if (side1 != side2) {
                overlay2IntersectBoundary(line->x1, line->y1, line->x2,
                                          line->y2, &intersectionX,
                                          &intersectionY);
                if (side1 == wantedSide) {
                    overlay2AppendLine(line->x1, line->y1, intersectionX,
                                       intersectionY, line->value1,
                                       line->value2);
                } else {
                    overlay2AppendLine(intersectionX, intersectionY, line->x2,
                                       line->y2, line->value1, line->value2);
                }
            } else if (side1 == wantedSide) {
                overlay2AppendLine(line->x1, line->y1, line->x2, line->y2,
                                   line->value1, line->value2);
            }
        } else {
            if (gOverlay2BoundaryAxis == 0) {
                xSide = 0;
                if (line->x1 < line->x2) {
                    xSide = 1;
                }
                side1 = xSide;
            } else {
                ySide = 0;
                if (line->y1 < line->y2) {
                    ySide = 1;
                }
                side1 = ySide;
            }
            if (side1 == wantedSide) {
                overlay2AppendLine(line->x1, line->y1, line->x2, line->y2,
                                   line->value1, line->value2);
            }
        }
        line++;
    }
    output->count = gOverlay2LineCount - output->start;
    return remaining + 1;
}
