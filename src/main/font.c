/*
 * Resident font and dialogue-window code -- ROM 0x4BC40-0x4E1E0
 * (VRAM 0x8004B040-0x8004D5E0).
 *
 * PROVENANCE -- the translation-unit identity, candidate function names,
 * declarations, and struct starting point come from Jet Force Gemini's
 * public decompilation, src/font.c and src/font.h. JFG is a permitted
 * published retail-derived decomp under docs/CLEANROOM.md. Mickey's own
 * instructions decide every field, body, boundary, and final name here.
 *
 * The boundary is supported at both ends rather than by a whole-object match:
 * JFG's first font.c function, fontSetWindow0, is byte-identical at 0x4BC40
 * (7 unmasked words, ROM-wide unique); its final fontYSpacing shape is the
 * leaf at 0x4E1C0; and the next function is the separate osCreatePiManager.
 *
 * Flags: -O2 -mips2 -32, inherited from the measured src/main rule.
 */

#include "game/font.h"

#pragma GLOBAL_ASM("asm/nonmatchings/main/font/func_8004B040.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/font/func_8004B064.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/font/fontSetWindowNoise.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/font/func_8004B0A4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/font/func_8004B0B8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/font/func_8004B0DC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/font/func_8004B0F8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/font/func_8004B13C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/font/func_8004B1DC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/font/func_8004BA8C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/font/func_8004BB44.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/font/func_8004BBE0.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/font/func_8004BBFC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/font/fontWindowFontColour.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/font/func_8004BC84.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/font/func_8004BCC4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/font/func_8004BF64.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/font/func_8004BFB0.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/font/func_8004BFD8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/font/func_8004C000.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/font/func_8004C0C4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/font/func_8004C140.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/font/func_8004C200.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/font/func_8004C5A4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/font/func_8004C690.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/font/func_8004C8D8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/font/func_8004D32C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/font/func_8004D39C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/font/func_8004D40C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/font/func_8004D5C0.s")
