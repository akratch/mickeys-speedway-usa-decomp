/*
 * Fast3D/RCP task and clear helpers -- ROM 0x2F400-0x30CD0.
 *
 * PROVENANCE -- the TU identity and descriptive names are adapted from Jet
 * Force Gemini's public decompilation, src/rcpFast3d.c. Mickey's exact
 * rcpFast3d/rcpInit skeleton anchors, ordered init helpers and RCP call graph
 * establish the boundary. The bodies below remain Mickey GLOBAL_ASM.
 */

#include "PR/ultratypes.h"

#pragma GLOBAL_ASM("asm/nonmatchings/main/rcpFast3d/rcpFast3d.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/rcpFast3d/rcpWaitDP.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/rcpFast3d/rcpSetScreenColour.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/rcpFast3d/bgdraw_fillcolour.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/rcpFast3d/func_8002EBD4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/rcpFast3d/func_8002EBE0.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/rcpFast3d/rcpClearZBuffer.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/rcpFast3d/rcpClearScreen.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/rcpFast3d/rcpInitDp.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/rcpFast3d/rcpInitDpNoSize.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/rcpFast3d/rcpInitSp.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/rcpFast3d/rcpInit.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/rcpFast3d/func_8002F618.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/rcpFast3d/func_8002FB34.s")
