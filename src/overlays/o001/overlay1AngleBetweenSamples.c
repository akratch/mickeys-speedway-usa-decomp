#include "PR/ultratypes.h"

typedef struct Overlay1SampleState {
    u8 pad0[0x37E];
    u8 selector;
} Overlay1SampleState;

/* Pinned DKR v77/v80 and JFG scans classify overlay 1 as no donor. */
extern Overlay1SampleState *gOverlay1SampleState;
extern void overlay1SampleReloc(f32 *x, f32 *y, s32 selector, f32 scale);
extern s32 overlay1SampleAngleReloc(f32 x, f32 y);

s32 overlay1AngleBetweenSamples(f32 unusedX, f32 unusedY) {
    f32 firstX;
    f32 firstY;
    f32 secondX;
    f32 secondY;

    overlay1SampleReloc(&firstX, &firstY, gOverlay1SampleState->selector, 1.0f);
    overlay1SampleReloc(&secondX, &secondY, gOverlay1SampleState->selector,
                        2.5f);
    return (s16)(overlay1SampleAngleReloc(firstX - secondX,
                                          firstY - secondY) - 0x8000);
}
