#include "PR/ultratypes.h"

typedef struct Overlay1PathPoint {
    s16 x;
    s16 y;
} Overlay1PathPoint;

typedef struct Overlay1Path {
    Overlay1PathPoint *points;
    u32 count;
} Overlay1Path;

/* Fresh pinned DKR v77/v80 and JFG scans found no Overlay 1 donor. */
extern Overlay1Path *overlay1GetPathReloc(u8 selector);
extern s32 overlay1AngleReloc(f32 y, f32 x);
extern s32 overlay1AngleDifferenceReloc(s16 first, s16 second);
extern f32 overlay1TrigXReloc(s32 angle);
extern f32 overlay1TrigYReloc(s32 angle);

#ifdef NON_MATCHING
void overlay1BendPathPoint(s16 *x, s16 *y, u8 index, u8 selector) {
    Overlay1PathPoint *next, *previous, *current;
    Overlay1Path *path;
    s16 midpointAngle, secondAngle, firstAngle;
    volatile u8 localIndex;
    s32 nextIndex, previousIndex, currentIndex;

    localIndex = index;
    path = overlay1GetPathReloc(selector);
    index = localIndex;
    current = &path->points[index];
    if (index != 0) {
        currentIndex = index;
        previousIndex = index - 1;
    } else {
        previousIndex = path->count - 1;
        currentIndex = 0;
    }
    previous = &path->points[previousIndex];
    if (currentIndex >= path->count) {
        nextIndex = 0;
    } else {
        nextIndex = currentIndex + 1;
    }
    next = &path->points[nextIndex];
    firstAngle = (s16)(overlay1AngleReloc((f32)(current->y - previous->y),
                                         (f32)(current->x - previous->x)) -
                       0x8000);
    secondAngle = (s16)(overlay1AngleReloc((f32)(current->y - next->y),
                                          (f32)(current->x - next->x)) -
                        0x8000);
    midpointAngle = firstAngle +
        (overlay1AngleDifferenceReloc(firstAngle, secondAngle) >> 1);
    *x = (s16)((f32)*x - overlay1TrigXReloc(midpointAngle) * 50.0f);
    *y = (s16)((f32)*y - overlay1TrigYReloc(midpointAngle) * 50.0f);
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay1BendPathPoint/func_overlay_001_F0007730_1853B10.s")
#endif
