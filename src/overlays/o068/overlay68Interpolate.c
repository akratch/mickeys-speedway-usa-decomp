#include "PR/ultratypes.h"

f32 overlay68Interpolate(
    f32 weight,
    s32 previous,
    s32 current,
    s32 next,
    s32 following,
    s32 wrap,
    f32 *derivative,
    s32 linear) {
    f32 fPrevious;
    f32 fCurrent;
    f32 fNext;
    f32 fFollowing;
    s32 difference;
    u32 period;

    if (wrap != 0) {
        difference = current - previous;
        period = 0x10000;
        if (difference >= 0x8001) {
            current -= period;
            next -= period;
            following -= period;
        } else if (difference < -0x8000) {
            current += period;
            next += period;
            following += period;
        }

        difference = next - current;
        period = 0x10000;
        if (difference >= 0x8001) {
            next -= period;
            following -= period;
        } else if (difference < -0x8000) {
            next += period;
            following += period;
        }

        difference = following - next;
        if (difference >= 0x8001) {
            following -= period;
        } else if (difference < -0x8000) {
            following += period;
        }
    }

    if (linear != 0) {
        fCurrent = (f32)current;
        if (derivative != NULL) {
            *derivative = 0.0f;
        }
        return (((f32)next - fCurrent) * weight) + fCurrent;
    }

    if (derivative != NULL) {
        fPrevious = (f32)previous;
        fCurrent = (f32)current;
        fNext = (f32)next;
        fFollowing = (f32)following;
        *derivative = (((((-0.5f * fPrevious) + (1.5f * fCurrent) + (-1.5f * fNext) +
            (0.5f * fFollowing)) * 3.0f * weight) +
            (2.0f * (fPrevious + (-2.5f * fCurrent) + (2.0f * fNext) +
            (-0.5f * fFollowing)))) * weight) +
            ((-0.5f * fPrevious) + (0.5f * fNext));
    }

    fPrevious = (f32)previous;
    fCurrent = (f32)current;
    fNext = (f32)next;
    fFollowing = (f32)following;
    return (((((((-0.5f * fPrevious) + (1.5f * fCurrent) + (-1.5f * fNext) +
        (0.5f * fFollowing)) * weight) +
        (fPrevious + (-2.5f * fCurrent) + (2.0f * fNext) +
        (-0.5f * fFollowing))) * weight) +
        ((-0.5f * fPrevious) + (0.5f * fNext))) * weight) + fCurrent;
}
