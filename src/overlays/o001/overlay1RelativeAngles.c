#include "PR/ultratypes.h"

typedef struct Overlay1Position {
    u8 pad0[0xC];
    f32 x;
    u8 pad10[4];
    f32 y;
} Overlay1Position;

/* Pinned DKR v77/v80 and JFG scans classify overlay 1 as no donor. */
extern Overlay1Position *gOverlay1Position;
extern s32 overlay1AngleReloc(f32 x, f32 y);

s32 overlay1RelativeAngleA(f32 x, f32 y) {
    s32 angle;
    f32 deltaX;
    f32 deltaY;

    deltaX = x - gOverlay1Position->x;
    deltaY = y - gOverlay1Position->y;
    angle = overlay1AngleReloc(deltaX, deltaY);
    return (s16)(angle + 0x8000);
}

s32 overlay1RelativeAngleB(f32 x, f32 y) {
    s32 angle;
    f32 deltaX;
    f32 deltaY;

    deltaX = x - gOverlay1Position->x;
    deltaY = y - gOverlay1Position->y;
    angle = overlay1AngleReloc(deltaX, deltaY);
    return (s16)(angle + 0x8000);
}
