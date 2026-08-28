#include "overlays/o100/motion.h"

/* The runtime relocation table supplies the release routine. It is not the
 * incompatible overlay-local initializer whose unloaded JAL bits are also 0. */
extern void overlay100ReleaseMotionReloc(Overlay100Motion *motion);
extern f32 gOverlay100GravityReloc[];

/* Workbench: structure-mismatch, 51 raw differing words, first mismatch +0x3C.
 * Exact 95/96-instruction loop shape and FP schedule; the candidate is one word short.
 * Structural gap: target's countdown-carrier move at +0xC0; rest is allocation/relocation. */
#ifdef NON_MATCHING
Overlay100Motion *overlay100UpdateMotion(Overlay100Motion *motion, s32 step) {
    s32 oldPhase, nextPhase, remaining, count;
    Overlay100Vec3 *velocity, *oldFrame, *newFrame;
    f32 timeStep, blendScale, gravityStep, verticalBias;

    if (motion == 0) return 0;
    motion->remaining -= step;
    if (motion->remaining <= 0) {
        overlay100ReleaseMotionReloc(motion);
        return 0;
    }
    blendScale = step;
    oldPhase = motion->phase;
    nextPhase = oldPhase + 1;
    if (nextPhase >= 3) nextPhase = 0;
    timeStep = blendScale;
    motion->phase = nextPhase;
    if (motion->bank < 3) motion->bank++;
    else {
        motion->nextBank++;
        if (motion->nextBank >= 3) motion->nextBank = 0;
    }
    count = motion->count;
    velocity = motion->velocity;
    oldFrame = motion->frames[oldPhase];
    newFrame = motion->frames[nextPhase];
    remaining = count - 1;
    if (count != 0) {
        gravityStep = -(gOverlay100GravityReloc[1] * timeStep);
        verticalBias = ((f32)(step + 1) * gravityStep) * 0.5f;
        do {
            newFrame->x = oldFrame->x + velocity->x * blendScale;
            newFrame->y = oldFrame->y + velocity->y * blendScale + verticalBias;
            newFrame->z = oldFrame->z + velocity->z * blendScale;
            velocity->y += gravityStep;
            velocity++;
            oldFrame++;
            newFrame++;
        } while (remaining--);
    }
    return motion;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o100/overlay100UpdateMotion/func_overlay_100_F000038C_18DB0B4.s")
#endif
