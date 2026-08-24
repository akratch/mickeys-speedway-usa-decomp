/*
 * Display-list disassembler -- ROM 0x459C0-0x465B0 (VRAM 0x80044DC0).
 *
 * PROVENANCE: the translation-unit identity and descriptive function names
 * are adapted from Jet Force Gemini's public decompilation, src/diRcp.c.
 * Mickey's opcode-name strings, helper call graph and source-order-equivalent
 * function sequence establish the correspondence. The two unnamed unpackers
 * retain Mickey address names. The bodies remain extracted assembly.
 */

#include "PR/ultratypes.h"

typedef void Gfx;

#pragma GLOBAL_ASM("asm/nonmatchings/main/diRcp/diRcpPrintDL.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diRcp/func_800453C4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diRcp/func_80045400.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diRcp/diRcpVertex.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diRcp/diRcpReserved1.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diRcp/diRcpMatrix.s")
/* PROVENANCE: body adapted from JFG src/diRcp.c::diRcpReserved0. */
s32 diRcpReserved0(Gfx *dList, char *name) {
    return 8;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/diRcp/diRcpReserved2.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diRcp/diRcpMoveMem.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diRcp/diRcpDisplayList.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diRcp/diRcpStrNameMacro.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diRcp/diRcpPrimColor.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diRcp/diRcpColor.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diRcp/diRcpDmaOffsets.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diRcp/diRcpMoveWd.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diRcp/diRcpStrName.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diRcp/diRcpOtherMode.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diRcp/diRcpGeometryMode.s")
