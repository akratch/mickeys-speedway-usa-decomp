/*
 * Shadow buffers and projection -- ROM 0x16A90-0x18FF0.
 *
 * PROVENANCE -- the TU attribution and the five descriptive names below are
 * borrowed from Jet Force Gemini's public retail-derived decompilation:
 * src/shadows.c and asm/nonmatchings/shadows/*.s.  They are supported here at
 * tier B by the same call graph and at tier D by function order and masked
 * instruction shape.  This is not a whole-object tier-A match; Mickey's ROM
 * remains the authority for every body.
 *
 * The matched leaf bodies below were reconstructed from Mickey's own
 * instructions and globals; no JFG body has been adapted.  The remaining
 * pragmas preserve the original ROM bytes.  JFG's address-placeholder helper
 * names are deliberately not imported.
 */

#include "PR/ultratypes.h"

extern s32 D_80079458;
extern void *D_80079410[4];
extern void *D_80079420[4];
extern void *D_80079430[4];
extern void mmFree(void *ptr);

#pragma GLOBAL_ASM("asm/nonmatchings/main/shadows/shadowInitBuffers.s")
/* PROVENANCE -- adapted from JFG's public asm/nonmatchings/shadows/shadowFreeBuffers.s. */
void shadowFreeBuffers(void) {
    if (D_80079410[0] != NULL) {
        mmFree(D_80079410[0]);
        D_80079410[0] = NULL;
    }
    if (D_80079420[0] != NULL) {
        mmFree(D_80079420[0]);
        D_80079420[0] = NULL;
    }
    if (D_80079430[0] != NULL) {
        mmFree(D_80079430[0]);
        D_80079430[0] = NULL;
    }
}
void shadowChangeBuffer(void) {
    D_80079458 ^= 1;
}
void shadowGetBuffers(s32 arg0, void **arg1, void **arg2, void **arg3) {
    s32 index = D_80079458;

    if (arg0 & 2) {
        index += 2;
    }
    *arg1 = D_80079410[index];
    *arg2 = D_80079420[index];
    *arg3 = D_80079430[index];
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/shadows/shadowGenerate.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/shadows/func_80016890.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/shadows/func_80017140.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/shadows/func_80017660.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/shadows/func_80017BCC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/shadows/func_800180B4.s")
