/*
 * libultra libc string routines -- ROM 0x72370-0x72410 (VRAM 0x80071770).
 *
 * First C translation unit in this project; it exists to prove the IDO 5.3 +
 * asm-processor build path end to end.  The TU was identified by exact byte
 * identity with the built libultra `string.c` object of the public DKR
 * decompilation, and its extent (0xA0 bytes, including a 12-byte tail pad) is
 * what fixed the split point in mickey.us.yaml.
 *
 * Build flags: -O2 -mips2 -32.  This deviates from the project default
 * (-mips1) and the deviation is forced by evidence: with -mips1 IDO will not
 * emit branch-likely instructions and the TU compiles to 0x90 bytes that do
 * not match the ROM; with -mips2 it emits the `bnel`/`beql` forms the ROM
 * actually contains and reproduces all 0xA0 bytes exactly.  See the per-file
 * MIPSISET override in the Makefile.
 */

#include "PR/ultratypes.h"
#include "string.h"

#pragma GLOBAL_ASM("asm/nonmatchings/libultra/string/memcpy.s")

#pragma GLOBAL_ASM("asm/nonmatchings/libultra/string/strlen.s")

#pragma GLOBAL_ASM("asm/nonmatchings/libultra/string/strchr.s")
