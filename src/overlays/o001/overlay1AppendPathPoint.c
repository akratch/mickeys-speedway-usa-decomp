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

extern f32 sqrtf(f32 value);
extern s32 overlay1UpdateValueCache(s16 x, s16 y, f32 value);
extern s16 overlay1AnchorX;
extern s16 overlay1AnchorY;

void overlay1AppendPathPoint(Overlay1PathState *state, s16 x, s16 y,
                             u8 primary, u8 secondary) {
    register s32 pointX = x;
    register s32 pointY = y;
    u8 index = state->count;
    s16 dx = pointX - state->x[index];
    s16 dy;
    s16 anchorX;
    s16 anchorDx;

    dy = pointY - state->y[index];
    state->count = index + 1;
    state->x[state->count] = pointX;
    state->y[state->count] = pointY;
    state->primary[state->count] = primary;
    state->secondary[state->count] = secondary;
    state->length += sqrtf((f32)((dx * dx) + (dy * dy)));
    state->flags = (state->flags & ~3) | 1;

    if ((state->count >= 2) &&
        (overlay1UpdateValueCache(pointX, pointY, state->length) == 0)) {
        state->flags &= ~3;
        return;
    }

    anchorX = overlay1AnchorX;
    anchorDx = pointX - anchorX;
    if ((pointX == anchorX) && (pointY == overlay1AnchorY)) {
        state->anchorDistanceSquared = 0;
    } else {
        s16 anchorDy = pointY - overlay1AnchorY;
        state->anchorDistanceSquared =
            (anchorDx * anchorDx) + (anchorDy * anchorDy);
    }
}
