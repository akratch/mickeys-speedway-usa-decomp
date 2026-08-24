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

extern void func_800453C4(Gfx *dList, s32 *w0_24_31, s32 *w0_16_23,
                          s32 *w0_0_15, s32 *w1);

#pragma GLOBAL_ASM("asm/nonmatchings/main/diRcp/diRcpPrintDL.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diRcp/func_800453C4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diRcp/func_80045400.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diRcp/diRcpVertex.s")
/* PROVENANCE: body adapted from JFG src/diRcp.c::diRcpReserved1. */
s32 diRcpReserved1(Gfx *dList) {
    s32 w0_24_31;
    s32 w0_16_23;
    s32 w0_0_15;
    s32 w1;
    s32 pad[4];

    func_800453C4(dList, &w0_24_31, &w0_16_23, &w0_0_15, &w1);
    return 8;
}
/* PROVENANCE: body adapted from JFG src/diRcp.c::diRcpMatrix. */
s32 diRcpMatrix(Gfx *dList) {
    s32 w0_24_31;
    s32 w0_16_23;
    s32 w0_0_15;
    s32 w1;
    s32 pad[8];

    func_800453C4(dList, &w0_24_31, &w0_16_23, &w0_0_15, &w1);
    return 8;
}
/* PROVENANCE: body adapted from JFG src/diRcp.c::diRcpReserved0. */
s32 diRcpReserved0(Gfx *dList, char *name) {
    return 8;
}
/* PROVENANCE: body adapted from JFG src/diRcp.c::diRcpReserved2. */
s32 diRcpReserved2(Gfx *dList) {
    s32 w0_24_31;
    s32 w0_16_23;
    s32 w0_0_15;
    s32 w1;
    s32 pad[2];

    func_800453C4(dList, &w0_24_31, &w0_16_23, &w0_0_15, &w1);
    return 8;
}
/* PROVENANCE: body adapted from JFG src/diRcp.c::diRcpMoveMem. */
s32 diRcpMoveMem(Gfx *dList) {
    s32 w0_24_31;
    s32 w0_16_23;
    s32 w0_0_15;
    s32 w1;
    s32 pad[2];

    func_800453C4(dList, &w0_24_31, &w0_16_23, &w0_0_15, &w1);
    return 8;
}
/* PROVENANCE: body adapted from JFG src/diRcp.c::diRcpDisplayList. */
s32 diRcpDisplayList(Gfx *dList) {
    s32 w0_24_31;
    s32 w0_16_23;
    s32 w0_0_15;
    s32 w1;
    s32 pad[2];

    func_800453C4(dList, &w0_24_31, &w0_16_23, &w0_0_15, &w1);
    return 8;
}
/* PROVENANCE: body adapted from JFG src/diRcp.c::diRcpStrNameMacro. */
s32 diRcpStrNameMacro(Gfx *dList, char *name, char *macroName) {
    return 8;
}
/* PROVENANCE: body adapted from JFG src/diRcp.c::diRcpPrimColor. */
s32 diRcpPrimColor(Gfx *dList) {
    return 8;
}
/* PROVENANCE: body adapted from JFG src/diRcp.c::diRcpColor. */
s32 diRcpColor(Gfx *dList, char *name, char *macroName) {
    return 8;
}
/* PROVENANCE: body adapted from JFG src/diRcp.c::diRcpDmaOffsets. */
s32 diRcpDmaOffsets(Gfx *dList, char *name) {
    if (dList) {
    }
    return 8;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/diRcp/diRcpMoveWd.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diRcp/diRcpStrName.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diRcp/diRcpOtherMode.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diRcp/diRcpGeometryMode.s")
