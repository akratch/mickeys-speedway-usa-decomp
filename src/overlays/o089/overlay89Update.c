#include "PR/ultratypes.h"

typedef struct Overlay89Position {
    s16 x;
    s16 y;
} Overlay89Position;

typedef struct Overlay89Motion {
    u8 pad0;
    u8 flags;
    u8 pad2[6];
    u8 frozen;
    u8 pad9[7];
    s16 xVelocity;
    s16 yVelocity;
} Overlay89Motion;

/*
 * DKR v77/v80 and JFG have no exact donor or matching damping/threshold
 * source shape for this exact two-axis motion update.
 */
void overlay89Update(Overlay89Position *position, Overlay89Motion *motion,
                     s32 updateRate) {
    if (motion->flags & 1) {
        if (motion->frozen == 0) {
            motion->xVelocity -= motion->xVelocity >> 5;
            if (motion->xVelocity >= -0x20 && motion->xVelocity < 0x21) {
                motion->xVelocity = 0;
            }
        }
        position->x += motion->xVelocity * updateRate;
    }

    if (motion->flags & 2) {
        if (motion->frozen == 0) {
            motion->yVelocity -= motion->yVelocity >> 5;
            if (motion->yVelocity >= -0x20 && motion->yVelocity < 0x21) {
                motion->yVelocity = 0;
            }
        }
        position->y += motion->yVelocity * updateRate;
    }
}
