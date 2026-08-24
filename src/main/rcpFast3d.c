/*
 * Fast3D/RCP task and clear helpers -- ROM 0x2F400-0x30CD0.
 *
 * PROVENANCE -- the TU identity and descriptive names are adapted from Jet
 * Force Gemini's public decompilation, src/rcpFast3d.c. Mickey's exact
 * rcpFast3d/rcpInit skeleton anchors, ordered init helpers and RCP call graph
 * establish the boundary. Adapted C bodies are identified in docs/modules.md;
 * all remaining functions stay as Mickey GLOBAL_ASM.
 */

#include "PR/ultratypes.h"

extern u8 D_8007A3A0;
extern u8 D_8007A3A4;
extern u8 D_8007A3A8;
extern u32 D_8007A3B0;

#pragma GLOBAL_ASM("asm/nonmatchings/main/rcpFast3d/rcpFast3d.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/rcpFast3d/rcpWaitDP.s")
void rcpSetScreenColour(u8 red, u8 green, u8 blue) {
    D_8007A3A0 = red;
    D_8007A3A4 = green;
    D_8007A3A8 = blue;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/rcpFast3d/bgdraw_fillcolour.s")
void func_8002EBD4(u32 value) {
    D_8007A3B0 = value;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/rcpFast3d/func_8002EBE0.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/rcpFast3d/rcpClearZBuffer.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/rcpFast3d/rcpClearScreen.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/rcpFast3d/rcpInitDp.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/rcpFast3d/rcpInitDpNoSize.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/rcpFast3d/rcpInitSp.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/rcpFast3d/rcpInit.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/rcpFast3d/func_8002F618.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/rcpFast3d/func_8002FB34.s")
