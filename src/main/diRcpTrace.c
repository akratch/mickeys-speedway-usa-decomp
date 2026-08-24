/*
 * Display-list trace buffer -- ROM 0x45760-0x459C0 (VRAM 0x80044B60).
 *
 * PROVENANCE: the translation-unit identity and function identities are adapted
 * from Jet Force Gemini's public decompilation, src/diRcpTrace.c. Mickey's
 * own call graph, placement immediately before diRcp.c, and the four-function
 * order establish the correspondence; JFG is a permitted published decomp
 * under docs/CLEANROOM.md. Externally referenced functions retain their
 * Mickey address labels until their callers are decompiled. The bodies remain
 * Mickey's extracted assembly.
 */

#include "PR/ultratypes.h"

extern s32 D_8007CFC8;
extern s32 D_800D4A90[];
extern s32 D_8007CFC0;
extern s32 D_8007CFC4;
extern s32 func_8002B280(s32 size, s32 tag);

void diRcpTraceInit(void) {
    D_8007CFC0 = func_8002B280(0x4B0, 0x8F);
    D_8007CFC4 = func_8002B280(0x4B0, 0x8F);
}
void func_80044B9C(void) {
    D_8007CFC8 = 1 - D_8007CFC8;
    D_800D4A90[D_8007CFC8] = 0;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/diRcpTrace/func_80044BC8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diRcpTrace/func_80044C94.s")
