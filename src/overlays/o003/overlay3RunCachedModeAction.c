#include "PR/ultratypes.h"
typedef struct Overlay3Object { s16 heading; u8 pad02[0x0A]; f32 x; f32 y; f32 z; } Overlay3Object;
typedef struct Overlay3Control { u8 pad000[1]; s8 chanceIndex; u8 pad002[0x198]; u8 mode; u8 pad19B[0x1F2]; u8 cachedIndex; } Overlay3Control;
extern Overlay3Object **overlay3GetSearchObjectsReloc(s32 *count);
extern s32 overlay3PlanarAngleReloc(f32 dx, f32 dz);
extern s32 overlay3AngleDeltaReloc(s16 heading, s32 angle);
extern s32 overlay1EncodeAngleReloc(s32 one, s32 shiftedAngle);
extern s32 overlay2CheckPathReloc(f32 ax, f32 az, f32 bx, f32 bz, s32 angleValue, s32 zero, s32 minusOne, s32 mask);
extern s32 overlay3RandomRangeReloc(s32 low, s32 high);
extern void overlay36Mode3ActionReloc(Overlay3Object *object);
extern void overlay36Mode4ActionReloc(Overlay3Object *object);
extern void overlay36Mode6ActionReloc(Overlay3Object *object);
extern void overlay36Mode7ActionReloc(Overlay3Object *object);
extern u8 gOverlay3ModeChance[];
/*
 * Plateau (2026-08-25, r4 pass): the 113-word candidate has exact size, a
 * 0x58-byte frame, and 43 differing words beginning at +0x7C.  The target
 * carries the packed angle directly into the encode call and performs the
 * compensating boolean handoff after the path check; current C places that
 * copy before the encode call, cascading through the switch.  The neutral
 * 119-combination flag sweep and ten directed variants covering register
 * qualification, boolean spelling, dead-web priority, existing-local reuse,
 * statement grouping, and prototype shape did not leave this basin.
 */
#ifdef NON_MATCHING
s32 overlay3RunCachedModeAction(Overlay3Object *anchor, Overlay3Control *control) {
    s32 count; Overlay3Object **objects; Overlay3Object *target;
    s32 packedAngle; s32 angle; s32 valid; s32 encoded; s32 random;
    objects = overlay3GetSearchObjectsReloc(&count);
    if (control->cachedIndex == 0x7F) return 0;
    target = objects[control->cachedIndex];
    if (target == anchor) return 0;
    packedAngle = overlay3AngleDeltaReloc(anchor->heading,
        overlay3PlanarAngleReloc(anchor->x - target->x, anchor->z - target->z)) << 16;
    angle = packedAngle >> 16;
    valid = angle >= -1999;
    if (valid) {
        valid = angle < 2000;
        if (valid) {
            encoded = overlay1EncodeAngleReloc(1, packedAngle);
            valid = overlay2CheckPathReloc(anchor->x, anchor->z, target->x, target->z,
                                           encoded, 0, -1, 0xFFFF) == 0;
        }
    }
    switch (control->mode) {
        case 3:
            if (valid) {
                random = overlay3RandomRangeReloc(1, 100);
                if (gOverlay3ModeChance[control->chanceIndex] < random) overlay36Mode3ActionReloc(anchor);
            }
            break;
        case 4:
            if (valid) {
                random = overlay3RandomRangeReloc(1, 100);
                if (gOverlay3ModeChance[control->chanceIndex] < random) overlay36Mode4ActionReloc(anchor);
            }
            break;
        case 6: overlay36Mode6ActionReloc(anchor); break;
        case 7: overlay36Mode7ActionReloc(anchor); break;
    }
    return 0;
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o003/overlay3RunCachedModeAction/func_overlay_003_F00000B8_1859DE8.s")
#endif
