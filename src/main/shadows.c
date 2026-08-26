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
extern u8 *D_80079410[4];
extern u8 *D_80079414[];
extern u8 *D_80079420[4];
extern u8 *D_80079424[];
extern u8 *D_80079430[4];
extern u8 *D_80079434[];
extern u8 *D_80079440;
extern s32 D_800CB278;
extern s32 D_800CB27C;
extern s32 D_800CB280;
extern s32 D_800CB284;
extern s32 D_800CB288;
extern void *func_8002B280(s32 size, s32 tag);
extern void mmFree(void *ptr);

#ifdef NON_MATCHING
/* PROVENANCE: adapted from JFG's public asm/nonmatchings/shadows/shadowInitBuffers.s; Mickey globals are authoritative.
 * Workbench: relocation-symbol-mismatch; 75/75 instructions and frame exact, with two endpoint sites naming D_80079440.
 * Levers: array extent and typed sentinel forms; the latter changes schedule/addend, so D_80079434+0xC remains the blocker. */
void shadowInitBuffers(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4) {
    s32 i;
    s32 stride0;
    s32 stride1;
    s32 stride2;

    D_800CB284 = arg0;
    D_800CB288 = arg1;
    D_800CB278 = arg2;
    D_800CB27C = arg3;
    stride0 = arg2 * 10;
    D_800CB280 = arg4;
    D_80079410[0] = func_8002B280(stride0 * 4, 0x8D);
    stride1 = arg3 * 16;
    D_80079420[0] = func_8002B280(stride1 * 4, 0x8D);
    stride2 = arg4 * 8;
    D_80079430[0] = func_8002B280(stride2 * 4, 0x8D);

    for (i = 0; i < 3; i++) {
        D_80079414[i] = D_80079414[i - 1] + stride0;
        D_80079424[i] = D_80079424[i - 1] + stride1;
        D_80079434[i] = D_80079434[i - 1] + stride2;
    }
    D_80079458 = 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/shadows/shadowInitBuffers.s")
#endif
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
