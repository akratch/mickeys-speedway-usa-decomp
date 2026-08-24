/*
 * Resident model loading and instance management -- ROM 0x20020-0x21DA0
 * (VRAM 0x8001F420-0x800211A0).
 *
 * The working TU is identified from three exact masked-skeleton matches to
 * Jet Force Gemini's built src/models.c.o, the first at the existing yaml
 * boundary, plus the allocator, texture, and matrix call graph of the rest of
 * the block. It is not a whole-object match; docs/modules.md section 3.4
 * records the evidence and keeps uncertain JFG correspondences as comments
 * rather than adopting names.
 *
 * PROVENANCE -- JFG's public decomp was consulted for the models.c function
 * order, names, prototypes, and structure vocabulary. No body is adapted in
 * this all-GLOBAL_ASM split. Any body later adapted from JFG must retain a
 * point-of-use PROVENANCE note, and Mickey's own bytes remain authoritative.
 *
 * Flags: -O2 -mips2 -32, via the measured src/main/ Makefile rule.
 */

#include "PR/ultratypes.h"

#pragma GLOBAL_ASM("asm/nonmatchings/main/models/func_8001F420.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/models/func_8001F45C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/models/func_8001F520.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/models/func_8001FB64.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/models/func_8001FBCC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/models/func_8001FC50.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/models/func_80020184.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/models/func_80020278.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/models/func_800203E0.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/models/func_800204B8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/models/func_80020564.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/models/func_80020570.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/models/func_8002057C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/models/func_80020AD4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/models/func_80020B10.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/models/func_80020D8C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/models/func_80020E4C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/models/func_80021010.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/models/func_8002109C.s")
