#include "PR/ultratypes.h"

typedef struct Overlay1PathState {
    s16 x[32];
    s16 y[32];
    u8 primary[32];
    u8 secondary[32];
    u8 count;
    u8 flags;
    u8 padC2[2];
    f32 length;
    u32 anchorDistanceSquared;
} Overlay1PathState;

typedef struct Overlay1Point {
    s16 x;
    s16 y;
} Overlay1Point;

typedef struct Overlay1Entry {
    Overlay1Point *points;
} Overlay1Entry;

typedef struct Overlay1TraceResult {
    s16 x;
    s16 y;
    u16 base;
    u16 first;
    u16 second;
    u16 secondary;
    s32 changed;
} Overlay1TraceResult;

extern s32 overlay2TracePath(f32 x, f32 y, f32 anchorX, f32 anchorY,
                             void *arg5, Overlay1TraceResult *result,
                             u8 primary, u8 secondary);
extern Overlay1Entry *overlay1GetEntry(u16 index);
extern Overlay1PathState *overlay1CloneRecord(Overlay1PathState *source);
extern void overlay1AppendPathPoint(Overlay1PathState *state, s32 x, s32 y,
                                    u8 primary, u8 secondary);
extern s16 overlay1AnchorX;
extern s16 overlay1AnchorY;
extern void *gOverlay1SubmitArg5;
extern s32 gOverlay1PoolExhausted;

/* DKR v77/v80 and JFG have no exact donor for this bounded path advance. */
s32 overlay1AdvancePath(Overlay1PathState *state) {
    s16 currentX;
    s16 currentY;
    Overlay1Entry *entry;
    Overlay1TraceResult result;
    Overlay1PathState *child;
    register s32 nextX;
    u8 count;

    count = state->count;
    currentX = state->x[count];
    currentY = state->y[count];

    state->flags = (state->flags & ~3) |
                   (*(u16 *)&state->count & 1);
    if (count >= 31) {
        return 1;
    }

    if (!overlay2TracePath((f32)currentX, (f32)currentY,
                           (f32)overlay1AnchorX, (f32)overlay1AnchorY,
                           gOverlay1SubmitArg5, &result,
                           state->primary[count], state->secondary[count]) ||
        ((result.x == overlay1AnchorX) && (result.y == overlay1AnchorY))) {
        overlay1AppendPathPoint(state, overlay1AnchorX, overlay1AnchorY, 0xFF, 0);
        return 1;
    }

    entry = overlay1GetEntry(result.secondary);
    nextX = result.x;
    if ((currentX != nextX) || (currentY != result.y)) {
        overlay1AppendPathPoint(state, nextX, result.y,
                                *((u8 *)&result + 5), result.secondary);
        if (result.changed != 0) {
            state->flags = (state->flags & ~3) |
                           ((*(u16 *)&state->count | 2) & 3);
        }
    }

    if ((result.base != result.first) && (gOverlay1PoolExhausted == 0)) {
        child = overlay1CloneRecord(state);
        if (child != NULL) {
            overlay1AppendPathPoint(child, entry->points[result.first].x,
                                    entry->points[result.first].y,
                                    *((u8 *)&result + 7), result.secondary);
            child->flags = (child->flags & ~3) |
                           ((*(u16 *)&child->count | 2) & 3);
        }
    }

    if ((result.base != result.second) && (gOverlay1PoolExhausted == 0)) {
        child = overlay1CloneRecord(state);
        if (child != NULL) {
            overlay1AppendPathPoint(child, entry->points[result.second].x,
                                    entry->points[result.second].y,
                                    *((u8 *)&result + 9), result.secondary);
            child->flags = (child->flags & ~3) |
                           ((*(u16 *)&child->count | 2) & 3);
        }
    }
    return 1;
}
