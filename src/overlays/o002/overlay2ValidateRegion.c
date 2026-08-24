#include "PR/ultratypes.h"

typedef struct Overlay2Line {
    f32 x1;
    f32 y1;
    f32 x2;
    f32 y2;
    u16 value1;
    u16 value2;
} Overlay2Line;

typedef struct Overlay2Region {
    s32 boundaryAxis;
    f32 boundaryValue;
    u8 pad8[8];
    u16 start;
    u16 count;
} Overlay2Region;

extern Overlay2Line *gOverlay2Lines;
extern s32 func_8002A910(f32 y, f32 x);
extern s32 func_8002AA0C(s32 current, s32 target);

/* Pinned DKR v77/v80 and JFG scans found no exact region-validator donor. */
s32 overlay2ValidateRegion(Overlay2Region *region) {
    Overlay2Line *line;
    Overlay2Line *next;
    s32 remaining;
    s32 angle;
    s32 nextAngle;
    s16 connectorAngle;

    if (region->count != 1) {
        line = &gOverlay2Lines[region->start];
        remaining = region->count;
        while (remaining--) {
            angle = (s16)func_8002A910(line->y2 - line->y1,
                                       line->x2 - line->x1);
            if (remaining != 0) {
                next = line + 1;
            } else {
                next = &gOverlay2Lines[region->start];
            }

            nextAngle = (s16)func_8002A910(next->y2 - next->y1,
                                           next->x2 - next->x1);
            if ((line->x2 != next->x1) || (line->y2 != next->y1)) {
                connectorAngle =
                    (s16)func_8002A910(next->y1 - line->y2,
                                       next->x1 - line->x2);
                if ((func_8002AA0C(angle, connectorAngle) < 0) ||
                    (func_8002AA0C(connectorAngle, nextAngle) < 0)) {
                    return 0;
                }
            } else if (func_8002AA0C(angle, nextAngle) < 0) {
                return 0;
            }

            line = next;
        }
    }

    return 1;
}
