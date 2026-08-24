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

#pragma GLOBAL_ASM("asm/nonmatchings/main/diRcpTrace/diRcpTraceInit.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diRcpTrace/func_80044B9C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diRcpTrace/func_80044BC8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/diRcpTrace/func_80044C94.s")
