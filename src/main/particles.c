/*
 * Resident particle system -- ROM 0x3D5F0-0x43470.
 *
 * PROVENANCE: The translation-unit attribution and the non-placeholder names
 * below come from Jet Force Gemini's public decompilation, src/particles.c and
 * its built particles.c.o. Mickey's own function order, masked-skeleton
 * similarity, and call graph establish the correspondence. JFG address-based
 * placeholders are not imported; Mickey's existing placeholders remain.
 * Adapted bodies carry a point-of-use PROVENANCE note.
 */

#include "PR/ultratypes.h"

extern f32 D_8007C8F8;
extern f32 D_8007C8F0;

#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/reset_particles.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_8003CA20.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_8003CB3C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_8003CCE4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_8003CD28.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_8003CE10.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_8003D25C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_8003D4FC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/partInitTrigger.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/partInitTriggerSPPos.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/partInitTriggerPos.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_8003E730.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_8003E7B8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_8003E8D8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_8003EB08.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_8003EC8C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/partObjFreeTriggers.s")
/* PROVENANCE: body adapted from JFG src/particles.c:partAdjustScaling. */
void partAdjustScaling(f32 scale) {
    D_8007C8F8 = scale;
}
void func_8003EDD4(f32 value) {
    D_8007C8F0 = value;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_8003EDE0.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/partUpdateTriggers.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_8003EF80.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_8003F154.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_8003F5F8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/partModelObjEmitModelPart.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_8003FB98.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_8004054C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_80040740.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_80040878.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_80040B88.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_80041040.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_80041388.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_80041530.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_80041C50.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_80041CE4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_80041F48.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_80041FEC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_800420E0.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_800421F4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_8004233C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/func_800423EC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/partUpdateParticles.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/partDraw.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/particles/partNullifyCircularParticleParents.s")
