/*
 * libultra: OR bits into the global hardware interrupt mask.
 *
 * ROM 0x75080-0x750D0 (VRAM 0x80074480). Byte-identical to Jet Force Gemini's
 * built libultra `os/setglobalintmask.c` object, so the file boundary is
 * measured rather than guessed -- see the provenance note in
 * symbol_addrs.us.txt. Perfect Dark's and Banjo-Kazooie's builds carry the
 * same bytes.
 *
 * NOT MATCHED. The body is
 *
 *     register u32 saveMask = __osDisableInt();
 *     __OSGlobalIntMask |= mask;
 *     __osRestoreInt(saveMask);
 *
 * and at -O1 -g3 -mips2 -32 this IDO reproduces the ROM's 19 instructions
 * except for one: the ROM fills the first `jal`'s delay slot with the
 * callee-save `sw s0, 0x18(sp)`, and this compiler emits that store before the
 * `jal` and leaves the slot empty, for 20 instructions. Every other word,
 * including the frame size, the s0 allocation and the two separate %hi/%lo
 * pairs on __OSGlobalIntMask, agrees. None of the twenty-two
 * {-O0,-O1,-O2} x {-g,-g1,-g2,-g3,none} x {-mips1,-mips2} combinations closes
 * that one slot. Left as assembly rather than committed one instruction wrong.
 */

#pragma GLOBAL_ASM("asm/nonmatchings/libultra/setglobalintmask/__osSetGlobalIntMask.s")
