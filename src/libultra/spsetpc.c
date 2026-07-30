/*
 * libultra __osSpSetPc -- ROM 0x74C20-0x74C60 (VRAM 0x80074020).
 *
 * DELIBERATELY NOT DECOMPILED. This file's job is to keep the asm-processor
 * `#pragma GLOBAL_ASM` path exercised by every `gmake verify`.
 *
 * Without it that path has no coverage at all: src/libultra/string.c is now
 * fully C, so nothing would assemble a nonmatchings .s, nothing would need
 * include/asm_processor_prelude.inc, and the GLOBAL_ASM half of the build
 * could rot silently until the next task hit it -- at which point it would be
 * debugged at the same time as that task's first relocations, i.e. two
 * untested variables at once.
 *
 * The TU was chosen for being the smallest thing that does the job: one
 * function, 0x40 bytes, a boundary splat had already isolated on its own, and
 * a .text byte-identical to DKR's built spsetpc.c object.
 *
 * If you decompile this, scaffold another all-GLOBAL_ASM TU in the same
 * commit. There are plenty of candidates left in the libultra corridor.
 */

#pragma GLOBAL_ASM("asm/nonmatchings/libultra/spsetpc/__osSpSetPc.s")
