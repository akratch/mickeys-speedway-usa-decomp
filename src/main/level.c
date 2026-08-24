#include "ultra64.h"

/*
 * Resident level lifecycle and metadata, ROM 0x263F0-0x27760.
 *
 * PROVENANCE: the TU attribution and function names are adapted from Jet
 * Force Gemini's published src/level.c and built src/level.c.o, a permitted
 * public retail-derived decomp under docs/CLEANROOM.md. Mickey's ordered call
 * graph independently establishes the correspondence (tier B).
 */

extern s32 D_800CF3C4;
extern u8 D_800CF420[];
extern u8 *D_800CF3C8;

#pragma GLOBAL_ASM("asm/nonmatchings/main/level/levelGetCounts.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/level/levelNGetType.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/level/levelGetTune.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/level/levelGetWorld.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/level/levelGetRegionNo.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/level/levelGetScreenMode.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/level/levelGetBlurEffect.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/level/levelGetGfxIndex.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/level/levelInit.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/level/levelTunePlay.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/level/levelUpdateColourCycling.s")

/* PROVENANCE: body adapted from JFG src/level.c; Mickey byte identity is decisive. */
u8 *levelGetColourCycling(void) {
    return D_800CF420;
}

/* PROVENANCE: body adapted from JFG src/level.c; Mickey byte identity is decisive. */
s32 levelGetNumber(void) {
    return D_800CF3C4;
}

#pragma GLOBAL_ASM("asm/nonmatchings/main/level/levelGetType.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/level/levelGetCamera.s")

/* PROVENANCE: body adapted from JFG src/level.c; Mickey byte identity is decisive. */
u8 *levelGetLevel(void) {
    return D_800CF3C8;
}

#pragma GLOBAL_ASM("asm/nonmatchings/main/level/levelGetName.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/level/levelFreeAll.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/level/levelGetNextOfWorld.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/level/levelGetPrevOfWorld.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/level/levelInitRegionFlags.s")
