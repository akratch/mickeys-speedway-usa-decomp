#include "PR/ultratypes.h"

typedef struct Overlay8MotionRow {
    u8 firstSelector;
    u8 secondSelector;
    u8 pad002[2];
    s16 firstScale;
    s16 secondScale;
} Overlay8MotionRow;

typedef struct Overlay8MotionAnchor {
    s16 helperInput;
    u8 pad002[0x0A];
    f32 x;
    u8 pad010[4];
    f32 y;
} Overlay8MotionAnchor;

typedef struct Overlay8MotionState {
    u8 pad000;
    s8 rowIndex;
    u8 pad002[0x191];
    u8 outsideLatch;
    f32 outsideValue;
    u8 pad198[0x24C];
    Overlay8MotionAnchor *target;
    s16 primary;
    s16 secondary;
} Overlay8MotionState;

extern const Overlay8MotionRow D_2230[];
extern s16 *gOverlay8Buffer;

extern s32 overlay8MeasureDirectionReloc(f32 deltaX, f32 deltaY);
extern s32 overlay8ConvertDirectionReloc(s32 anchorValue,
                                         s32 directionCode);
extern f32 overlay8ApproachMotionReloc(s32 difference,
                                       f32 doubledSecondary, f32 limit);

void overlay8UpdateMotionOutput(Overlay8MotionAnchor *anchor,
                                Overlay8MotionState *state,
                                f32 inputScale) {
    const Overlay8MotionRow *row;
    s32 span;
    Overlay8MotionAnchor *target;
    f32 delta;
    struct {
        f32 first;
        f32 second;
    } scales;

    row = &D_2230[state->rowIndex];
    target = state->target;
    span = row->firstScale + row->secondScale;

    if (target == NULL) {
        delta = 0.0f;
    } else {
        delta = (f32) overlay8ConvertDirectionReloc(
            anchor->helperInput,
            overlay8MeasureDirectionReloc(anchor->x - target->x,
                                          anchor->y - target->y));
    }

    if (span == 0) {
        return;
    }

    if ((f32) span < delta) {
        delta = (f32) span;
    } else if (delta < (f32) -span) {
        delta = (f32) -span;
    }

    if ((8000.0f < delta) || (delta < -8000.0f)) {
        if (state->outsideLatch == 0) {
            state->outsideLatch = 1;
            state->outsideValue = 0.04f;
        }
    } else if (state->outsideLatch == 1) {
        state->outsideLatch = 0;
    }

    if (delta != (f32) state->primary) {
        register f32 step;

        step = overlay8ApproachMotionReloc(
            (s32) (delta - (f32) state->primary),
            (f32) state->secondary * 2.0f, 800.0f);
        state->secondary = (s16) (s32) ((f32) state->secondary + step);
        state->primary = (s16) (s32) (
            (f32) state->primary +
            (f32) state->secondary * inputScale * 0.5f);
    }

    scales.first = (f32) row->firstScale * (1.0f / (f32) span);
    scales.second = (f32) row->secondScale * (1.0f / (f32) span);

    *gOverlay8Buffer = (s16) ((u32) row->firstSelector * 3U + 1U);
    gOverlay8Buffer++;
    *gOverlay8Buffer =
        (s16) (s32) ((f32) state->primary * scales.first);
    gOverlay8Buffer++;
    *gOverlay8Buffer = (s16) ((u32) row->secondSelector * 3U + 1U);
    gOverlay8Buffer++;
    *gOverlay8Buffer =
        (s16) (s32) ((f32) state->primary * scales.second);
    gOverlay8Buffer++;
}
