#include "PR/ultratypes.h"

typedef struct Overlay2Line {
    f32 x1;
    f32 y1;
    f32 x2;
    f32 y2;
    u16 value1;
    u16 value2;
} Overlay2Line;

extern Overlay2Line *gOverlay2Lines;
extern s32 gOverlay2LineCount;

/* Pinned DKR v77/v80 and JFG scans found no line-buffer donor. */
void overlay2AppendLine(f32 x1, f32 y1, f32 x2, f32 y2, u16 value1,
                        u16 value2) {
    if ((x1 != x2) || (y1 != y2)) {
        /* IDO schedules the endpoint load before the cached count address only when this original macro-like group shares a source line. */
        do { if (gOverlay2LineCount >= 0xC00) { while (1) { } } gOverlay2Lines[gOverlay2LineCount].x1 = x1; gOverlay2Lines[gOverlay2LineCount].y1 = y1; gOverlay2Lines[gOverlay2LineCount].x2 = x2; gOverlay2Lines[gOverlay2LineCount].y2 = y2; } while (0);
        gOverlay2Lines[gOverlay2LineCount].value1 = value1;
        gOverlay2Lines[gOverlay2LineCount].value2 = value2;
        gOverlay2LineCount++;
    }
}
