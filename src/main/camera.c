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
 * The matched functions below carry adapted JFG bodies, each with its own
 * PROVENANCE note at the point of use; everything else in this split is
 * still GLOBAL_ASM.
 *
 * Flags: -O2 -mips2 -32, from the existing src/main/ compilation rule.
 */

#include "PR/ultratypes.h"
#include "game/gameVi.h"
#include "game/math.h"
#include "n_audio/mbi.h"

typedef struct {
    s16 yRotation;
    s16 xRotation;
    s16 zRotation;
    u8 pad06[6];
    f32 x;
    f32 y;
    f32 z;
} CameraTransform;

typedef struct {
    CameraTransform transform;
    u8 pad18[0x14];
    f32 fov;
    f32 shakeX;
    f32 shakeY;
    f32 shakeZ;
    u8 pad3C[0xE];
    s16 pitchOffset;
    u8 stateA;
    u8 stateB;
    u8 pad4E[6];
} Camera;

typedef struct {
    s16 attackEnd;
    s16 sustainEnd;
    s16 totalEnd;
    s16 timer;
    s32 magnitude;
} CameraShake;

extern u8 D_80079F94;
extern s32 D_80079F8C;
extern u8 D_80079FA0[];
extern s32 D_800CEC84;
extern s32 D_800CEC88;
extern f32 D_800CEC8C;
extern f32 D_800CEC90;
extern u8 D_80079FA8[];
extern f32 D_80079FB0[];
extern Mtx D_800CED60[];
extern MtxF D_800CEC98;
extern CameraTransform D_800CEC68;
extern u16 D_800CEC94;
extern MtxF D_800CED18;
extern Mtx D_800CF160;
extern s32 D_800CEC60;
extern s32 D_800CEC64;
extern MtxF D_800CF1A0;
extern MtxF D_800CF1E0;
extern f32 D_800CF2A0;
extern Camera D_800CEA20[];
extern CameraShake D_800CEC18[];

void mtxf_mul(MtxF lhs, MtxF rhs, MtxF dest);
void mtxf_to_mtx(MtxF src, Mtx *dest);
void func_8002AB78(CameraTransform *transform, MtxF matrix);
void func_8002AE10(CameraTransform *transform, MtxF matrix);
extern s32 levelInitRegionFlags(void);
extern void func_80021504(f32 fov, s32 force);
extern void func_80021FB0(s32 mode, s32 camNo, s32 *x1, s32 *y1,
                          u32 *x2, u32 *y2);
extern void func_80022C58(Gfx **dlist, u32 halfWidth, u32 halfHeight,
                          u32 centerX, u32 centerY, s32 regionFlags);
extern void func_80022794(Gfx **dlist, Mtx **mtx);

#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/camInit.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80021438.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80021444.s")
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camUseShake. */
void camUseShake(void) {
    D_800CEC84 = 1;
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camIgnoreShake. */
void camIgnoreShake(void) {
    D_800CEC84 = 0;
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camGetFOV. */
f32 camGetFOV(void) {
    return D_800CF2A0;
}
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
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camGetMode. */
s32 camGetMode(void) {
    return D_800CEC60;
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camSetMode. */
s32 camSetMode(s32 mode) {
    if ((mode < 0) || (mode >= 4)) {
        mode = 0;
    }
    D_800CEC60 = mode;
    if (D_800CEC60 < D_800CEC64) {
        D_800CEC64 = 0;
    }
    return mode + 1;
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camGetNo. */
s32 camGetNo(void) {
    return D_800CEC64;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80021970.s")
/*
 * PROVENANCE: name and role from JFG's public decomp,
 * src/camera.c:camSetNo; body reconstructed from Mickey-only evidence.
 */
void camSetNo(s32 camNo) {
    D_800CEC64 = camNo;
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camGetWaterLine. */
u8 camGetWaterLine(s32 camNo) {
    return D_80079FA8[camNo];
}
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
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camSetView. */
void func_800221E8(Gfx **dlist, Mtx **mtx) {
    u32 halfWidth;
    struct {
        u32 pad;
        u32 lry;
        u32 lrx;
        u32 uly;
        u32 ulx;
    } win;
    u32 halfHeight;
    s32 videoMode;

    func_80021FB0(D_800CEC60, D_800CEC64, (s32 *)&win.ulx,
                  (s32 *)&win.uly, &win.lrx, &win.lry);

    videoMode = viGetVideoMode();
    if ((videoMode == 2) || (videoMode == 3)) {
        halfWidth = 224;
        halfHeight = 168;
    } else {
        halfWidth = 160;
        halfHeight = 120;
    }

    if (D_800CEC60 >= 2) {
        halfWidth >>= 1;
        halfHeight >>= 1;
    }

    if (D_80079F8C != 0) {
        win.lrx >>= 1;
        win.lry >>= 1;
        halfWidth >>= 1;
        halfHeight >>= 1;
    }

    if (D_80079FA0[D_800CEC64] != 0) {
        halfWidth *= D_80079FB0[D_800CEC64];
        halfHeight *= D_80079FB0[D_800CEC64];
        gSPClipRatio((*dlist)++, FRUSTRATIO_1);
    }

    gDPSetScissor((*dlist)++, G_SC_NON_INTERLACE,
                  win.ulx, win.uly, win.lrx, win.lry);
    func_80022C58(dlist, halfWidth, halfHeight,
                  (win.lrx + win.ulx) >> 1, (win.lry + win.uly) >> 1,
                  levelInitRegionFlags());

    if (mtx != NULL) {
        func_80022794(dlist, mtx);
    }
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80022604.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80022610.s")
/*
 * PROVENANCE: adapted from JFG's public decomp,
 * src/camera.c:camGetPlayerProjMtx.
 */
void camGetPlayerProjMtx(s32 player, MtxF dest) {
    mtxf_mul(D_800CF1A0, D_800CEC98, dest);
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camSetProjMtx. */
void func_80022794(Gfx **dlist, Mtx **mtx) {
    Camera *camera;

    camera = &D_800CEA20[D_800CEC64];
    if ((D_800CF2A0 != camera->fov) ||
        (camera->stateA != camera->stateB) ||
        (D_80079FA0[D_800CEC64] != 0)) {
        func_80021504(camera->fov, 0);
    }

    if (dlist != NULL) {
        gSPPerspNormalize((*dlist)++, D_800CEC94);
    }

    D_800CEC68.yRotation = camera->transform.yRotation + 0x8000;
    D_800CEC68.xRotation = camera->transform.xRotation + camera->pitchOffset;
    D_800CEC68.zRotation = camera->transform.zRotation;
    D_800CEC68.x = -camera->transform.x;
    D_800CEC68.y = -camera->transform.y;
    D_800CEC68.z = -camera->transform.z;
    if (D_800CEC84) {
        D_800CEC68.x -= camera->shakeX;
        D_800CEC68.y -= camera->shakeY;
        D_800CEC68.z -= camera->shakeZ;
    }
    func_8002AE10(&D_800CEC68, D_800CF1A0);
    mtxf_mul(D_800CF1A0, D_800CEC98, D_800CED18);

    if (dlist != NULL) {
        mtxf_to_mtx(D_800CED18, *mtx);
        gSPMatrix((*dlist)++, (u32)*mtx + 0x80000000,
                  G_MTX_NOPUSH | G_MTX_MUL | G_MTX_MODELVIEW);
        (*mtx)++;
    }

    D_800CEC68.yRotation = -0x8000 - camera->transform.yRotation;
    D_800CEC68.xRotation =
        -(camera->transform.xRotation + camera->pitchOffset);
    D_800CEC68.zRotation = -camera->transform.zRotation;
    D_800CEC68.x = camera->transform.x;
    D_800CEC68.y = camera->transform.y;
    D_800CEC68.z = camera->transform.z;
    if (D_800CEC84) {
        D_800CEC68.x += camera->shakeX;
        D_800CEC68.y += camera->shakeY;
        D_800CEC68.z += camera->shakeZ;
    }
    func_8002AB78(&D_800CEC68, D_800CF1E0);
    mtxf_to_mtx(D_800CF1E0, &D_800CF160);
}
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
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camStopShakes. */
void camStopShakes(void) {
    s32 i;
    Camera *cam;
    CameraShake *shake;

    D_800CEC84 = 0;
    cam = D_800CEA20;
    shake = D_800CEC18;

    /* IDO: line-join preserves the target store/increment schedule. */
    for (i = 6; i--; cam++, shake++) { \
        cam->shakeX = 0.0f; \
        cam->shakeY = 0.0f; \
        cam->shakeZ = 0.0f; \
        shake->magnitude = 0; \
    }
}
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
