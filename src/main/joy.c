#include "ultra64.h"

/*
 * Resident controller input, ROM 0x25C20-0x263F0.
 *
 * PROVENANCE: the TU attribution and function names are adapted from Jet
 * Force Gemini's published src/joy.c and built src/controller.c.o, a permitted
 * public retail-derived decomp under docs/CLEANROOM.md. Mickey's ordered call
 * graph independently establishes the correspondence (tier B).
 */

extern u16 D_8007A0C8;
extern u8 D_800CF3B0[];
extern u8 D_800CF3B4[];

#pragma GLOBAL_ASM("asm/nonmatchings/main/joy/joyMessageQ.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/joy/joyInit.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/joy/joyRead.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/joy/joyResetMap.s")

/* PROVENANCE: body adapted from JFG src/joy.c; Mickey byte identity is decisive. */
void joyDisable(s32 player) {
    D_800CF3B4[player & 3] = FALSE;
}

/* PROVENANCE: body adapted from JFG src/joy.c; Mickey byte identity is decisive. */
void joyEnable(s32 player) {
    D_800CF3B4[player & 3] = TRUE;
}

#pragma GLOBAL_ASM("asm/nonmatchings/main/joy/joyCreateMap.s")

/* PROVENANCE: body adapted from JFG src/joy.c; Mickey byte identity is decisive. */
u8 joyGetController(s32 player) {
    return D_800CF3B0[player];
}

#pragma GLOBAL_ASM("asm/nonmatchings/main/joy/joyGetButtons.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/joy/joyGetPressed.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/joy/joyGetReleased.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/joy/joyGetStickX.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/joy/joyGetAbsX.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/joy/joyGetStickY.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/joy/joyGetAbsY.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/joy/joyClamp.s")

/* PROVENANCE: body adapted from JFG src/joy.c; Mickey byte identity is decisive. */
void joySetSecurity(void) {
    D_8007A0C8 = 0;
}

#pragma GLOBAL_ASM("asm/nonmatchings/main/joy/arithmeticFunction.s")

/* PROVENANCE: body adapted from JFG src/joy.c; Mickey byte identity is decisive. */
s32 joyCharVal(void) {
    return 1;
}
