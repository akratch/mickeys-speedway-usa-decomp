#include "PR/ultratypes.h"

extern s32 gOverlay14InputFlag0;
extern s32 gOverlay14InputFlag4;
extern s32 gOverlay14InputFlag8;
extern s32 gOverlay14PulseC;
extern s32 gOverlay14Pulse10;
extern s32 gOverlay14Pulse14;
extern s32 gOverlay14Pulse18;
extern s32 gOverlay14Timer1C;
extern s32 gOverlay14Timer20;

extern s32 overlay14ReadInput(s32 channel);

/* Plateau (batch 20): exact 0x20C; 65 words remain, first +0x34.
 * Four branch locals improved 78; pointer, axis, call, load, and 119 flag variants failed.
 * The 40-minute permuter's lower scores required invented wrappers and were rejected. */
#ifdef NON_MATCHING
void overlay14PrepareInputState(s32 step) {
    s32 first;
    s32 second;
    s32 vertical;
    s32 verticalPositive;
    s32 verticalNegative;
    s32 secondPositive;
    s32 secondNegative;

    first = overlay14ReadInput(0);
    second = overlay14ReadInput(0);
    vertical = overlay14ReadInput(0);

    gOverlay14InputFlag0 = first & 0x8000;
    gOverlay14InputFlag4 = first & 0x4000;
    gOverlay14InputFlag8 = first & 0x1000;
    gOverlay14PulseC = 0;
    gOverlay14Pulse10 = 0;
    gOverlay14Pulse14 = 0;
    gOverlay14Pulse18 = 0;

    if ((vertical >= -0x1E) && (vertical < 0x1F)) {
        gOverlay14Timer20 = 0;
    } else if (vertical >= 0x1F) {
        if (gOverlay14Timer20 > 0) {
            verticalPositive = gOverlay14Timer20 - step;
            gOverlay14Timer20 = verticalPositive;
            if (verticalPositive <= 0) {
                gOverlay14Timer20 = 0xA;
                gOverlay14PulseC = 1;
            }
        } else {
            gOverlay14Timer20 = 0x14;
            gOverlay14PulseC = 1;
        }
    } else if (vertical < -0x1E) {
        if (gOverlay14Timer20 < 0) {
            verticalNegative = gOverlay14Timer20 + step;
            gOverlay14Timer20 = verticalNegative;
            if (verticalNegative >= 0) {
                gOverlay14Timer20 = -0xA;
                gOverlay14Pulse10 = 1;
            }
        } else {
            gOverlay14Timer20 = -0x14;
            gOverlay14Pulse10 = 1;
        }
    }

    if ((second >= -0x1D) && (second < -0x1E)) {
        gOverlay14Timer1C = 0;
        return;
    }
    if (second >= 0x1F) {
        secondPositive = gOverlay14Timer1C - step;
        if (gOverlay14Timer1C > 0) {
            gOverlay14Timer1C = secondPositive;
            if (secondPositive <= 0) {
                gOverlay14Timer1C = 0xA;
                gOverlay14Pulse18 = 1;
            }
        } else {
            gOverlay14Timer1C = 0x14;
            gOverlay14Pulse18 = 1;
        }
    } else if (second < -0x1E) {
        secondNegative = gOverlay14Timer1C + step;
        if (gOverlay14Timer1C < 0) {
            gOverlay14Timer1C = secondNegative;
            if (secondNegative <= 0) {
                gOverlay14Timer1C = -0xA;
                gOverlay14Pulse14 = 1;
            }
        } else {
            gOverlay14Timer1C = -0x14;
            gOverlay14Pulse14 = 1;
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o014/overlay14PrepareInputState/func_overlay_014_F0000B5C_1870434.s")
#endif
