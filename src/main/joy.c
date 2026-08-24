#include "ultra64.h"

/*
 * Resident controller input, ROM 0x25C20-0x263F0.
 *
 * PROVENANCE: the TU attribution and function names are adapted from Jet
 * Force Gemini's published src/joy.c and built src/controller.c.o, a permitted
 * public retail-derived decomp under docs/CLEANROOM.md. Mickey's ordered call
 * graph independently establishes the correspondence (tier B).
 */

#pragma GLOBAL_ASM("asm/nonmatchings/main/joy/joyMessageQ.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/joy/joyInit.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/joy/joyRead.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/joy/joyResetMap.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/joy/joyDisable.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/joy/joyEnable.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/joy/joyCreateMap.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/joy/joyGetController.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/joy/joyGetButtons.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/joy/joyGetPressed.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/joy/joyGetReleased.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/joy/joyGetStickX.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/joy/joyGetAbsX.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/joy/joyGetStickY.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/joy/joyGetAbsY.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/joy/joyClamp.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/joy/joySetSecurity.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/joy/arithmeticFunction.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/joy/joyCharVal.s")
