/*
 * Shadow buffers and projection -- ROM 0x16A90-0x18FF0.
 *
 * PROVENANCE -- the TU attribution and the five descriptive names below are
 * borrowed from Jet Force Gemini's public retail-derived decompilation:
 * src/shadows.c and asm/nonmatchings/shadows/*.s.  They are supported here at
 * tier B by the same call graph and at tier D by function order and masked
 * instruction shape.  This is not a whole-object tier-A match; Mickey's ROM
 * remains the authority for every body.
 *
 * No body has been adapted yet.  These pragmas preserve the original ROM
 * bytes while the compiler-generated functions are reconstructed one at a
 * time.  JFG's address-placeholder helper names are deliberately not
 * imported.
 */

#pragma GLOBAL_ASM("asm/nonmatchings/main/shadows/shadowInitBuffers.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/shadows/shadowFreeBuffers.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/shadows/shadowChangeBuffer.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/shadows/shadowGetBuffers.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/shadows/shadowGenerate.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/shadows/func_80016890.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/shadows/func_80017140.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/shadows/func_80017660.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/shadows/func_80017BCC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/shadows/func_800180B4.s")
