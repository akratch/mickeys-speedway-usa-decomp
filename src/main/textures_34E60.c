#include "PR/ultratypes.h"

/*
 * PROVENANCE: the texture-TU placement and the small mode setter below were
 * compared with Jet Force Gemini's public src/textures.c. Mickey's globals,
 * boundaries, and compiler output remain authoritative.
 */

extern s32 D_8007BD90;

#pragma GLOBAL_ASM("asm/nonmatchings/main/textures_34E60/func_80034260.s")

void func_800343F0(s32 flags) {
    D_8007BD90 |= flags;
}
