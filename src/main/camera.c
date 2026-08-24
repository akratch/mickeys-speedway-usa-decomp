/*
 * Resident camera -- ROM 0x21EE0-0x25C20 (VRAM 0x800212E0).
 *
 * The 69 functions cover camera state, user viewports, projection setup,
 * sprite/model matrices, projection helpers and screen shake. The final
 * function ends four bytes before the existing 16-byte subsegment boundary.
 * No function uses an odd single-precision floating-point register, so this
 * block has no handwritten-assembly classification under docs/modules.md 6.2.
 *
 * PROVENANCE DISCLOSURE. The camera TU identification and the adopted names in
 * this file were read from Jet Force Gemini's public retail-derived decomp,
 * src/camera.c, as permitted by docs/CLEANROOM.md. Mickey's own bytes decide
 * every name and body: docs/modules.md records which names are tier A, which
 * are tier B role/order arguments, and which functions remain unresolved.
 * This initial split contains no adapted function bodies, only GLOBAL_ASM.
 *
 * Flags: -O2 -mips2 -32, from the existing src/main/ compilation rule.
 */

#include "PR/ultratypes.h"

typedef union {
    s32 m[4][4];
    s64 force_structure_alignment;
} Mtx;

extern u8 D_80079F94;
extern u8 D_80079FA0[];
extern s32 D_800CEC84;
extern s32 D_800CEC88;
extern f32 D_800CEC8C;
extern f32 D_800CEC90;
extern u8 D_80079FA8[];
extern f32 D_80079FB0[];
extern Mtx D_800CED60[];

#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/camInit.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80021438.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80021444.s")
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camUseShake. */
void camUseShake(void) {
    D_800CEC84 = 1;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80021494.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_800214A0.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_800214AC.s")
/*
 * PROVENANCE: adapted from JFG's public decomp,
 * src/camera.c:camOverrideProjScales.
 */
void camOverrideProjScales(f32 scaleX, f32 scaleY) {
    D_800CEC8C = scaleX;
    D_800CEC90 = scaleY;
    D_800CEC88 = 1;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80021504.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80021718.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_800217AC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_800217B8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80021838.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80021918.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80021924.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80021964.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80021970.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80021994.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_800219A0.s")
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camSetWaterLine. */
void camSetWaterLine(s32 camNo, s32 waterLine) {
    if ((camNo >= 0) && (camNo < 4)) {
        D_80079FA8[camNo] = waterLine;
    }
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_800219D0.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80021B70.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80021BE4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80021C5C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80021C88.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80021DF4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80021EF0.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80021F68.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80021FB0.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_800221E8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80022604.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80022610.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/camGetPlayerProjMtx.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80022794.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80022A38.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80022A44.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80022A50.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80022B94.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80022C58.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80022D20.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80022E80.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80022FD4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80023598.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80023A08.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80023CCC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80023F84.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80024204.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_800242E0.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_8002442C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_800244EC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_800245EC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_8002460C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_8002462C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80024658.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80024664.s")
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camGetProjOrgMtx. */
Mtx *camGetProjOrgMtx(void) {
    return &D_800CED60[D_80079F94];
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_8002468C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80024698.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_800246A4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_800246B0.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80024834.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80024938.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80024978.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80024AC4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/camStopShakes.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80024BA0.s")
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camSetZoom. */
void camSetZoom(s32 camNo, f32 zoom) {
    if ((camNo >= 0) && (camNo < 6)) {
        D_80079FA0[camNo] = 1;
        D_80079FB0[camNo] = zoom;
    }
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80024D00.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80024ED8.s")
