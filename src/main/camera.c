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
 * Flags: -O2 -mips2 -32 -Wab,-r4300_mul; the projection-depth dot product
 * fixes the TU's multiply scheduler mode.
 */

#include "PR/ultratypes.h"
#include "PR/os_internal.h"
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
    f32 x;
    f32 y;
    f32 z;
} CameraPoint;

typedef struct {
    CameraTransform transform;
    f32 unk18;
    f32 unk1C;
    f32 unk20;
    f32 unk24;
    f32 unk28;
    f32 fov;
    f32 shakeX;
    f32 shakeY;
    f32 shakeZ;
    u8 unk3C;
    u8 unk3D;
    s16 unk3E;
    f32 unk40;
    u8 pad44[6];
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

typedef struct {
    u8 pad00[0xC];
    f32 x;
    f32 y;
    f32 z;
} CameraSpriteAnchor;

typedef struct {
    s16 angle;
    s16 frame;
    u16 pad04;
    u16 divisor;
    f32 transformScale;
    f32 matrixScale;
    f32 x;
    f32 y;
    f32 z;
    s32 frameCount;
    u8 *spriteData;
} CameraSprite;

typedef struct {
    s16 yRotation;
    s16 xRotation;
    s16 zRotation;
    u8 pad06[2];
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
} CameraScaledTransform;

typedef union {
    CameraScaledTransform transform;
    f64 align;
} CameraAlignedScaledTransform;

typedef struct {
    s32 flags;
    u8 pad04[0x30];
} CameraViewportFlags;

typedef struct {
    s32 x1;
    s32 y1;
    s32 x2;
    s32 y2;
    s32 posX;
    s32 posY;
    s32 width;
    s32 height;
    s32 scissorX1;
    s32 scissorY1;
    s32 scissorX2;
    s32 scissorY2;
    s32 flags;
} CameraViewport;

typedef struct {
    s8 state;
    u8 pad01[0x53];
} CameraState3D;

extern u8 D_80079F94;
extern s32 D_80079F8C;
extern f32 D_80079F60;
extern f32 D_80079F48;
extern f32 D_80079F4C[16];
extern CameraViewport D_80079C10[];
extern CameraViewportFlags D_80079C40[];
extern Vp D_80079D58[];
extern Vp D_80079E98[];
extern u8 D_80079F98[];
extern CameraState3D D_800CEA5D[];
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
extern CameraTransform D_80079F18;
extern u16 D_800CEC94;
extern MtxF D_800CED18;
extern MtxF D_800CECD8;
extern Mtx *D_800CED58;
extern Mtx D_800CF160;
extern s32 D_800CEC60;
extern s32 D_800CEC64;
extern s8 D_800CEC80;
extern MtxF D_800CF1A0;
extern MtxF D_800CF1E0;
extern MtxF D_800CF220;
extern f32 D_800CF2A4;
extern f32 D_800CF2A8;
extern f32 D_800CF2AC;
extern f32 D_800CF2B0;
extern MtxF D_800CF2B8;
extern MtxF D_800CF2F8;
extern MtxF D_800CF260;
extern f32 D_800CF2A0;
extern f32 D_800D2FB4;
extern f32 D_80081A1C;
extern f32 D_80081A2C;
extern f32 D_80081A40;
extern f32 D_80081A44;
extern f32 D_80081A48;
extern Camera D_800CEA20[];
extern CameraShake D_800CEC18[];
extern s32 D_8007C854;
extern s32 D_8007C85C;
extern u8 D_79FCC[];

void mtxf_mul(MtxF lhs, MtxF rhs, MtxF dest);
void mtxf_to_mtx(MtxF src, Mtx *dest);
void mtxf_translate_y(MtxF matrix, f32 y);
void matrixScale(f32 x, f32 y, f32 z, MtxF matrix);
void func_8004FAD0(MtxF matrix, u16 *perspNorm, f32 fovy, f32 aspect,
                   f32 nearPlane, f32 farPlane, f32 scale);
void mtxf_transform_point(MtxF matrix, f32 x, f32 y, f32 z,
                          f32 *outX, f32 *outY, f32 *outZ);
void func_80029AB8(MtxF matrix, f32 scale);
f32 func_8002A8BC(s32 angle);
f32 func_8002A8C0(s32 angle);
s16 Arctanf(f32 x, f32 y);
void func_8002AA50(CameraScaledTransform *transform, MtxF matrix);
void func_8002AB78(CameraTransform *transform, MtxF matrix);
void func_8002AE10(CameraTransform *transform, MtxF matrix);
void func_80024978(MtxF matrix);
void func_80034E54(Gfx **dlist, u8 *spriteData, s32 flags,
                   f32 frame, s32 alpha);
f32 sqrtf(f32 value);
extern s32 levelInitRegionFlags(void);
extern void func_80021504(f32 fov, s32 force);
void func_80021838(s32 x, s32 y, s32 z, s32 zRotation, s32 xRotation,
                   s32 yRotation);
extern void func_80021FB0(s32 mode, s32 camNo, s32 *x1, s32 *y1,
                          u32 *x2, u32 *y2);
extern void camSetViewport(Gfx **dlist, s32 halfWidth, s32 halfHeight,
                           s32 centerX, s32 centerY, s32 regionFlags);
extern void func_80022794(Gfx **dlist, Mtx **mtx);
void camStopShakes(void);

/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camInit. */
void camInit(void) {
    s32 i;

    D_800CEC64 = 0;
    D_800CEC60 = 0;
    D_800CEC80 = 0;
    _bzero(D_800CEA20, sizeof(Camera) * 6);
    for (i = 0; i < 6; i++) {
        D_800CEC64 = i;
        func_80021838(200, 200, 200, 0, 0, 180);
        D_800CEA20[D_800CEC64].fov = 60.0f;
    }
    camStopShakes();
    D_800CF2A0 = 60.0f;
    func_8004FAD0(D_800CEC98, &D_800CEC94, D_800CF2A0, 1.3333334f,
                  10.0f, D_80081A1C, 1.0f);
    D_80079F94 = (D_80079F94 + 1) & 0xF;
    mtxf_to_mtx(D_800CEC98, &D_800CED60[D_80079F94]);
}
f32 func_80021438(void) {
    return D_800D2FB4;
}
void func_80021444(s32 cameraIndex, s32 state) {
    if ((cameraIndex >= 0) && (cameraIndex < 6)) {
        D_80079F98[cameraIndex] = state;
        D_800CEA5D[cameraIndex].state = state;
    }
}
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
void func_800214AC(void) {
    D_800CEA20[D_800CEC64].stateA ^= 1;
}
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
#ifdef NON_MATCHING
/*
 * PROVENANCE: adapted from DKR's public decomp,
 * src/camera.c:cam_reset_fov.
 *
 * Plateau: the full flag lattice, six semantics-preserving source/type/address
 * variants, and a bounded two-worker permuter batch leave an exact 0x94-byte,
 * 37-instruction candidate with 11 positional words different from first
 * mismatch +0x4C. The remaining difference is temporary-register allocation
 * in the rotating matrix-slot update. The permuter's lower-scoring candidate
 * removed the required ring mask and invented a dead guard, so it was rejected.
 */
void func_80021718(void) {
    s32 index;
    s32 slot;

    func_8004FAD0(D_800CEC98, &D_800CEC94, 60.0f, 1.3333334f, 10.0f,
                  D_80081A2C, 1.0f);
    index = (D_80079F94 + 1) & 0xF;
    slot = index & 0xFF;
    D_80079F94 = index;
    mtxf_to_mtx(D_800CEC98,
                (Mtx *) ((slot << 6) + (u8 *) D_800CED60));
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80021718.s")
#endif
MtxF *func_800217AC(void) {
    return &D_800CF260;
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camDistance. */
f32 camDistance(f32 x, f32 y, f32 z) {
    Camera *cam;
    f32 dx;
    f32 dy;
    f32 dz;

    cam = &D_800CEA20[D_800CEC64];
    dx = x - cam->transform.x;
    dy = y - cam->transform.y;
    dz = z - cam->transform.z;
    return sqrtf((dx * dx) + (dy * dy) + (dz * dz));
}
/*
 * PROVENANCE: adapted from DKR's public decomp,
 * src/camera.c:camera_reset, with Mickey's own field layout and store order.
 */
void func_80021838(s32 x, s32 y, s32 z, s32 zRotation, s32 xRotation,
                   s32 yRotation) {
    Camera *camera;
    u8 *states;
    f32 floatX;
    f32 floatY;
    f32 sourceX;
    f32 floatZ;

    camera = &D_800CEA20[D_800CEC64];
    camera->transform.zRotation = zRotation * 182;
    floatX = x;
    sourceX = floatX;
    floatY = y;
    camera->transform.xRotation = xRotation * 182;
    floatZ = z;
    camera->transform.yRotation = yRotation * 182;
    states = D_80079F98;
    camera->pitchOffset = 0;
    camera->transform.x = sourceX;
    camera->transform.y = floatY;
    camera->unk18 = sourceX;
    camera->transform.z = floatZ;
    camera->unk1C = floatY;
    camera->unk20 = floatZ;
    camera->unk3C = 0;
    camera->shakeX = 0.0f;
    camera->shakeY = 0.0f;
    camera->shakeZ = 0.0f;
    camera->unk24 = 128.0f;
    camera->unk28 = 32.0f;
    camera->unk3D = states[D_800CEC64];
    camera->unk3E = -1;
    camera->unk40 = 0.0f;
}
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
Camera *func_80021970(s32 cameraIndex) {
    return &D_800CEA20[cameraIndex];
}
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
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camEnableUserView. */
void camEnableUserView(s32 camNo, s32 immediate) {
    CameraViewport *viewport;

    if (immediate != 0) {
        viewport = &D_80079C10[camNo];
        viewport->flags |= 1;
    } else {
        viewport = &D_80079C10[camNo];
        viewport->flags |= 2;
    }
    viewport->flags &= ~4;
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camDisableUserView. */
void camDisableUserView(s32 camNo, s32 immediate) {
    CameraViewport *viewport;

    if (immediate != 0) {
        viewport = &D_80079C10[camNo];
        viewport->flags &= ~1;
    } else {
        viewport = &D_80079C10[camNo];
        viewport->flags |= 4;
    }
    viewport->flags &= ~2;
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camIsUserView. */
s32 camIsUserView(s32 camNo) {
    return D_80079C40[camNo].flags & 1;
}
/*
 * PROVENANCE: adapted from DKR's public decomp,
 * src/camera.c:viewport_menu_set; JFG's public src/camera.c supplies the
 * camSetUserView role while Mickey supplies the video-size call and layout.
 */
void func_80021C88(s32 camNo, s32 x1, s32 y1, s32 x2, s32 y2) {
    s32 swap;
    struct {
        u32 height;
        u32 width;
    } videoSize;

    viGetCurrentSize((s32 *) &videoSize.width, (s32 *) &videoSize.height);
    if (x2 < x1) {
        swap = x1;
        x1 = x2;
        x2 = swap;
    }
    if (y2 < y1) {
        swap = y1;
        y1 = y2;
        y2 = swap;
    }
    if (((u32) x1 >= videoSize.width) || (x2 < 0) ||
        ((u32) y1 >= videoSize.height) ||
        (y2 < 0)) {
        D_80079C10[camNo].scissorX1 = 0;
        D_80079C10[camNo].scissorY1 = 0;
        D_80079C10[camNo].scissorX2 = 0;
        D_80079C10[camNo].scissorY2 = 0;
    } else {
        if (x1 < 0) {
            D_80079C10[camNo].scissorX1 = 0;
        } else {
            D_80079C10[camNo].scissorX1 = x1;
        }
        if (y1 < 0) {
            D_80079C10[camNo].scissorY1 = 0;
        } else {
            D_80079C10[camNo].scissorY1 = y1;
        }
        if ((u32) x2 >= videoSize.width) {
            D_80079C10[camNo].scissorX2 = videoSize.width - 1;
        } else {
            D_80079C10[camNo].scissorX2 = x2;
        }
        if ((u32) y2 >= videoSize.height) {
            D_80079C10[camNo].scissorY2 = videoSize.height - 1;
        } else {
            D_80079C10[camNo].scissorY2 = y2;
        }
    }
    D_80079C10[camNo].x1 = x1;
    D_80079C10[camNo].x2 = x2;
    D_80079C10[camNo].y1 = y1;
    D_80079C10[camNo].y2 = y2;
}
/*
 * PROVENANCE: adapted from JFG's public decomp,
 * src/camera.c:camSetUserViewSpecial.
 */
void camSetUserViewSpecial(s32 camNo, s32 posX, s32 posY, s32 width,
                           s32 height) {
    CameraViewport *viewport;

    if (posX != 0x8000) {
        viewport = &D_80079C10[camNo];
        viewport->posX = posX;
        viewport->flags |= 8;
    } else {
        viewport = &D_80079C10[camNo];
        viewport->flags &= ~8;
    }
    if (posY != 0x8000) {
        viewport->posY = posY;
        viewport->flags |= 0x10;
    } else {
        viewport->flags &= ~0x10;
    }
    if (width != 0x8000) {
        viewport->width = width;
        viewport->flags |= 0x20;
    } else {
        viewport->flags &= ~0x20;
    }
    if (height != 0x8000) {
        viewport->height = height;
        viewport->flags |= 0x40;
        return;
    }
    viewport->flags &= ~0x40;
}
/*
 * PROVENANCE: adapted from JFG's public decomp,
 * src/camera.c:camGetVisibleUserView.
 */
s32 camGetVisibleUserView(s32 camNo, s32 *x1, s32 *y1, s32 *x2, s32 *y2) {
    CameraViewport *viewport = &D_80079C10[camNo];

    *x1 = viewport->scissorX1;
    *x2 = viewport->scissorX2;
    *y1 = viewport->scissorY1;
    *y2 = viewport->scissorY2;
    if ((*x1 | *x2 | *y1 | *y2) == 0) {
        return 0;
    }
    return 1;
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camGetUserView. */
void camGetUserView(s32 camNo, s32 *x1, s32 *y1, s32 *x2, s32 *y2) {
    CameraViewport *viewport = &D_80079C10[camNo];

    *x1 = viewport->x1;
    *y1 = viewport->y1;
    *x2 = viewport->x2;
    *y2 = viewport->y2;
}
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
    camSetViewport(dlist, halfWidth, halfHeight, (win.lrx + win.ulx) >> 1,
                   (win.lry + win.uly) >> 1, levelInitRegionFlags());

    if (mtx != NULL) {
        func_80022794(dlist, mtx);
    }
}
void func_80022604(s32 arg0) {
    D_80079F8C = arg0;
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camSetScissor. */
void camSetScissor(Gfx **dlist) {
    u32 x1;
    u32 y1;
    u32 x2;
    u32 y2;

    func_80021FB0(D_800CEC60, D_800CEC64, (s32 *) &x1, (s32 *) &y1,
                  &x2, &y2);
    gDPSetScissor((*dlist)++, G_SC_NON_INTERLACE, x1, y1, x2, y2);
}
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
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camOrthoYAspect. */
void camOrthoYAspect(f32 aspect) {
    D_80079F60 = aspect;
}
void func_80022A44(f32 arg0) {
    D_80079F48 = arg0;
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camStandardOrtho. */
void camStandardOrtho(Gfx **dlist, Mtx **mtx) {
    u32 width;
    u32 height;
    Vp *viewport;

    viGetCurrentSize((s32 *) &width, (s32 *) &height);
    if (D_80079F8C != 0) {
        width >>= 1;
        height >>= 1;
        viewport = &D_80079E98[(D_800CEC64 & 3) | 4];
    } else {
        viewport = &D_80079D58[D_800CEC64 + 5];
    }
    viewport->vp.vscale[0] = width * 2;
    viewport->vp.vscale[1] = width * 2;
    viewport->vp.vtrans[0] = width * 2;
    viewport->vp.vtrans[1] = height * 2;
    gSPViewport((*dlist)++, (u32) viewport + 0x80000000);
    func_80024978(D_800CED18);
    mtxf_to_mtx(D_800CED18, *mtx);
    gSPMatrix((*dlist)++, (u32) *mtx + 0x80000000, 0);
    (*mtx)++;
}

/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camStandardPersp. */
void camStandardPersp(Gfx **dlist, Mtx **mtx) {
    gSPPerspNormalize((*dlist)++, D_800CEC94);
    func_8002AE10(&D_80079F18, D_800CF220);
    mtxf_mul(D_800CF220, D_800CEC98, D_800CED18);
    mtxf_to_mtx(D_800CED18, *mtx);
    gSPMatrix((*dlist)++, (u32) *mtx + 0x80000000, 0);
    (*mtx)++;
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camSetViewport. */
void camSetViewport(Gfx **dlist, s32 halfWidth, s32 halfHeight, s32 centerX,
                    s32 centerY, s32 regionFlags) {
    s32 camNo;
    Vp *viewport;

    camNo = D_800CEC64;
    if (D_80079F8C != 0) {
        viewport = &D_80079E98[camNo & 3];
    } else {
        viewport = &D_80079D58[camNo];
    }
    if (!(D_80079C40[camNo].flags & 1)) {
        viewport->vp.vtrans[0] = centerX * 4;
        viewport->vp.vtrans[1] = centerY * 4;
        viewport->vp.vscale[0] = halfWidth * 4;
        viewport->vp.vscale[1] = halfHeight * 4;
        if (regionFlags != 0) {
            viewport->vp.vscale[0] = -viewport->vp.vscale[0];
        }
    }
    gSPViewport((*dlist)++, (u32) viewport + 0x80000000);
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camResetView. */
void func_80022D20(Gfx **dlist) {
    u32 height;
    u32 width;

    D_800CEC64 = 4;
    viGetCurrentSize((s32 *) &width, (s32 *) &height);
    if (!(D_80079C40[D_800CEC64].flags & 1)) {
        gDPSetScissor((*dlist)++, G_SC_NON_INTERLACE, 0, 0, width - 1,
                      height - 1);
        camSetViewport(dlist, width >> 1, height >> 1, width >> 1,
                       height >> 1, 0);
    } else {
        camSetScissor(dlist);
        camSetViewport(dlist, 0, 0, 0, 0, 0);
    }
    D_800CEC64 = 0;
}
/* Mickey-only camera-relative billboard offset reconstruction. */
void func_80022E80(CameraScaledTransform *transform) {
    f32 x;
    f32 y;
    f32 z;
    CameraAlignedScaledTransform inverse;

    inverse.transform.yRotation = -transform->yRotation;
    inverse.transform.xRotation = -transform->xRotation;
    inverse.transform.zRotation = -transform->zRotation;
    inverse.transform.x = 0.0f;
    inverse.transform.y = 0.0f;
    inverse.transform.z = 0.0f;
    inverse.transform.scale = 1.0f;
    func_8002AA50(transform, D_800CF2B8);
    func_8002AE10((CameraTransform *) &inverse.transform, D_800CF2F8);
    D_800CF2B0 = 1.0f / transform->scale;
    x = D_800CEA20[D_800CEC64].transform.x - transform->x;
    y = D_800CEA20[D_800CEC64].transform.y - transform->y;
    z = D_800CEA20[D_800CEC64].transform.z - transform->z;
    mtxf_transform_point(D_800CF2F8, x, y, z, &x, &y, &z);
    D_800CF2A4 = x * D_800CF2B0;
    D_800CF2A8 = y * D_800CF2B0;
    D_800CF2AC = z * D_800CF2B0;
}
#ifdef NON_MATCHING
/*
 * PROVENANCE: JFG's public src/camera.c identifies the camDoSprite role;
 * this substantially different body is reconstructed from Mickey-only data.
 *
 * Plateau: after the full flag lattice and ten source/lifetime variants, the
 * best -Wab,-r4300_mul candidate has the target's 0xB0 frame and emits 366
 * instructions against 369. It differs in 297 positional words, beginning at
 * +0x2C where IDO places the three transformed-coordinate stack homes twelve
 * bytes above the target. Later expression scheduling leaves three missing
 * instructions, so the canonical build remains assembly-backed.
 */
void func_80022FD4(Gfx **dlist, Mtx **mtx, void *vertices,
                   CameraSpriteAnchor *anchor, f32 *opacity,
                   CameraSprite *sprite, s32 flags, s32 alpha) {
    register CameraSprite *spriteEarly;
    s32 angleProduct;
    s32 quadrant;
    f32 transformedX;
    f32 transformedY;
    f32 transformedZ;
    f32 localX;
    f32 localY;
    f32 localZ;
    f32 rotatedX;
    f32 rotatedZ;
    f32 cosine;
    f32 horizontal;
    CameraScaledTransform transform;
    f32 sine;
    s32 angle;
    s32 pitch;
    s32 frameStep;
    s32 wrappedFrame;
    u16 divisor;
    s32 color;
    Gfx *cmd;

    spriteEarly = sprite;
    transformedX = spriteEarly->x - anchor->x;
    transformedY = spriteEarly->y - anchor->y;
    transformedZ = spriteEarly->z - anchor->z;
    mtxf_transform_point(D_800CF2F8, transformedX, transformedY,
                         transformedZ, &transformedX, &transformedY,
                         &transformedZ);
    transformedX = transformedX * D_800CF2B0;
    transformedY = transformedY * D_800CF2B0;
    transformedZ = transformedZ * D_800CF2B0;
    localX = D_800CF2A4 - transformedX;
    localY = D_800CF2A8 - transformedY;
    localZ = D_800CF2AC - transformedZ;

    cosine = func_8002A8C0(sprite->angle);
    sine = func_8002A8BC(sprite->angle);
    rotatedX = (localX * sine) + (localZ * cosine);
    rotatedZ = (localZ * sine) - (localX * cosine);
    angle = Arctanf(rotatedX,
                    sqrtf((localY * localY) + (rotatedZ * rotatedZ)));
    cosine = -func_8002A8C0(Arctanf(rotatedX, rotatedZ));
    if (rotatedZ < 0.0f) {
        rotatedZ = -rotatedZ;
        cosine = -cosine;
        angle = -angle;
    }

    pitch = Arctanf(localY, rotatedZ);
    if (pitch >= 0x8001) {
        pitch += 0xFFFF0000;
    }

    divisor = sprite->divisor;
    quadrant = angle & 0x4000;
    frameStep = (s32)sprite->spriteData[0] / (s32)divisor;
    angle &= 0x3FFF;
    angleProduct = (s32)((f32)pitch * cosine);
    if (quadrant != 0) {
        angle = 0x3FFF - angle;
    }
    angle = (angle * frameStep) >> 14;
    if ((s32)divisor >= 2) {
        wrappedFrame = (u16)sprite->frame;
        while (wrappedFrame >= sprite->frameCount) {
            wrappedFrame -= sprite->frameCount;
        }
        angle += frameStep * (((s32)divisor * wrappedFrame) /
                              sprite->frameCount);
    }

    horizontal = sqrtf((localX * localX) + (localZ * localZ));
    transform.yRotation = Arctanf(localX, localZ);
    transform.xRotation = -Arctanf(localY, horizontal);
    transform.zRotation = angleProduct;
    transform.scale = sprite->transformScale;
    transform.x = transformedX;
    transform.y = transformedY;
    transform.z = transformedZ;
    func_8002AA50(&transform, D_800CF220);

    if (quadrant != 0) {
        D_800CF220[0][0] = -D_800CF220[0][0];
        D_800CF220[0][1] = -D_800CF220[0][1];
        D_800CF220[0][2] = -D_800CF220[0][2];
    }
    cosine = sprite->matrixScale;
    if (cosine != 1.0f) {
        func_80029AB8(D_800CF220, cosine);
    }
    mtxf_mul(D_800CF220, D_800CF2B8, D_800CECD8);
    mtxf_mul(D_800CECD8, D_800CED18, D_800CF220);
    mtxf_to_mtx(D_800CF220, *mtx);
    D_800CED58 = *mtx;

    if (flags & 4) {
        flags |= 1;
    } else {
        flags &= ~1;
    }

    if (D_8007C854 != 0) {
        if (opacity != NULL) {
            color = *opacity * D_8007C85C;
        } else {
            color = D_8007C85C;
        }
    } else {
        color = 255;
        if (opacity != NULL) {
            color = *opacity * 255.0f;
        }
    }

    color &= 0xFF;
    cmd = *dlist;
    *dlist = cmd + 1;
    cmd->words.w0 = 0xFA000000;
    cmd->words.w1 = (color << 24) | (color << 16) | (color << 8) |
                    (alpha & 0xFF);

    cmd = *dlist;
    *dlist = cmd + 1;
    cmd->words.w0 = 0x01020040;
    cmd->words.w1 = (u32)*mtx + 0x80000000;
    (*mtx)++;

    cmd = *dlist;
    *dlist = cmd + 1;
    cmd->words.w0 = (((((u32)D_79FCC & 6) | 8) & 0xFF) << 16) |
                    0x04000012;
    cmd->words.w1 = (u32)D_79FCC;

    func_80034E54(dlist, sprite->spriteData, flags & 0xF,
                  (f32)angle, alpha);

    cmd = *dlist;
    *dlist = cmd + 1;
    cmd->words.w1 = 0;
    cmd->words.w0 = 0xBC00000A;

    cmd = *dlist;
    *dlist = cmd + 1;
    cmd->words.w1 = -1;
    cmd->words.w0 = 0xFA000000;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80022FD4.s")
#endif
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80023598.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80023A08.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80023CCC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80023F84.s")
/*
 * PROVENANCE: adapted from JFG's public decomp,
 * src/camera.c:camPushFloatModelMtx.
 */
void camPushFloatModelMtx(Gfx **dlist, Mtx **mtx, MtxF matrix) {
    s32 i;
    s32 j;

    i = 0;
    do {
        j = 0;
        do {
            D_800CECD8[i][j] = matrix[i][j];
            j++;
        } while (j < 4);
        i++;
    } while (i < 4);
    mtxf_mul(matrix, D_800CED18, D_800CF260);
    mtxf_to_mtx(D_800CF260, *mtx);
    D_800CED58 = *mtx;
    gSPMatrix((*dlist)++, (u32) *mtx + 0x80000000, 1);
    (*mtx)++;
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camPushMuzzleMtx. */
void camPushMuzzleMtx(Gfx **dlist, Mtx **mtx, CameraPoint *point, MtxF matrix) {
    f32 outZ;
    f32 outY;
    f32 outX;
    MtxF temp;
    s32 i;
    s32 j;

    D_800CECD8[3][0] = 0.0f;
    D_800CECD8[3][1] = 0.0f;
    D_800CECD8[3][2] = 0.0f;
    mtxf_mul(matrix, D_800CECD8, temp);
    i = 0;
    do {
        j = 0;
        do {
            D_800CECD8[i][j] = temp[i][j];
            j++;
        } while (j < 4);
        i++;
    } while (i < 4);
    mtxf_transform_point(D_800CECD8, point->x, point->y, point->z,
                         &outX, &outY, &outZ);
    D_800CECD8[3][0] = point->x;
    D_800CECD8[3][1] = point->y;
    D_800CECD8[3][2] = point->z;
    mtxf_mul(D_800CECD8, D_800CED18, D_800CF260);
    mtxf_to_mtx(D_800CF260, *mtx);
    D_800CED58 = *mtx;
    gSPMatrix((*dlist)++, (u32) *mtx + 0x80000000, 1);
    (*mtx)++;
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camScaleModelMtx. */
void camScaleModelMtx(Gfx **dlist, Mtx **mtx, f32 scale) {
    if (scale != 1.0f) {
        matrixScale(scale, scale, scale, D_800CECD8);
        mtxf_mul(D_800CECD8, D_800CED18, D_800CF260);
        mtxf_to_mtx(D_800CF260, *mtx);
        D_800CED58 = *mtx;
        gSPMatrix((*dlist)++, (u32) *mtx + 0x80000000, 1);
        (*mtx)++;
    }
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camPushModelMtx. */
void camPushModelMtx(Gfx **dlist, Mtx **mtx, CameraScaledTransform *transform,
                     f32 scale, f32 scaleY) {
    func_8002AA50(transform, D_800CECD8);
    if (scaleY != 0.0f) {
        mtxf_translate_y(D_800CECD8, scaleY);
    }
    if (scale != 1.0f) {
        func_80029AB8(D_800CECD8, scale);
    }
    mtxf_mul(D_800CECD8, D_800CED18, D_800CF260);
    mtxf_to_mtx(D_800CF260, *mtx);
    D_800CED58 = *mtx;
    gSPMatrix((*dlist)++, (u32) *mtx + 0x80000000, 1);
    (*mtx)++;
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camRestoreModelMtx. */
void camRestoreModelMtx(Gfx **dlist) {
    {
        Gfx *cmd = (Gfx *)((*dlist)++);

        cmd->words.w1 = 0;
        cmd->words.w0 = 0xBC00000A;
    }
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camPopModelMtx. */
void camPopModelMtx(Gfx **dlist) {
    {
        Gfx *cmd = (Gfx *)((*dlist)++);

        cmd->words.w1 = 0;
        cmd->words.w0 = 0xBC00000A;
    }
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camGetPtr. */
Camera *camGetPtr(void) {
    return &D_800CEA20[D_800CEC64];
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camGetListPtr. */
Camera *camGetListPtr(void) {
    return D_800CEA20;
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camGetInvProjMtx. */
MtxF *camGetInvProjMtx(void) {
    return &D_800CF1E0;
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camGetProjOrgMtx. */
Mtx *camGetProjOrgMtx(void) {
    return &D_800CED60[D_80079F94];
}
MtxF *func_8002468C(void) {
    return &D_800CEC98;
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camGetRotationMtx. */
MtxF *camGetRotationMtx(void) {
    return &D_800CF1A0;
}
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camGetProjectionMtx. */
MtxF *camGetProjectionMtx(void) {
    return &D_800CED18;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_800246B0.s")

#ifdef NON_MATCHING
/*
 * PROVENANCE: name and role from JFG's public decomp,
 * src/camera.c:camReversePoint; body reconstructed from Mickey-only evidence.
 *
 * Plateau: the full flag lattice and a bounded two-worker permuter batch leave
 * a 66-instruction configured candidate against the 65-instruction target,
 * with 59 positional words different from first mismatch +0x0. The required
 * TU multiply scheduler uses a 0x28-byte frame and removes the target's dead
 * float spill; the same semantic body is byte-exact without that TU override,
 * which cannot be changed because camGetProjZ requires it.
 */
void func_80024834(f32 screenX, f32 screenY, f32 *x, f32 *y, f32 *z,
                   u8 transform) {
    Vp *viewport;
    f32 scale;
    f32 transY;

    viewport = &D_80079D58[D_800CEC64];
    scale = (*z * D_800CEC98[2][2]) * D_800CEC98[2][3];
    *x = (((f32) (viewport->vp.vtrans[0] >> 2) - screenX) * scale) /
         (D_800CEC98[0][0] * (f32) (viewport->vp.vscale[0] >> 2));
    transY = (f32) (viewport->vp.vtrans[1] >> 2);
    *y = ((screenY - transY) * scale) /
         (D_800CEC98[1][1] * (f32) (viewport->vp.vscale[1] >> 2));
    if (transform != 0) {
        mtxf_transform_point(D_800CF1E0, *x, *y, *z, x, y, z);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80024834.s")
#endif
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camGetProjZ. */
f32 camGetProjZ(f32 x, f32 y, f32 z) {
    f32 temp;
    f32 out;

    temp = D_800CF1A0[3][2];
    out = temp +
          (z * D_800CF1A0[2][2] +
           (y * D_800CF1A0[1][2] + D_800CF1A0[0][2] * (temp = x)));
    temp = y * D_800CF1A0[1][2] + x * D_800CF1A0[0][2];
    temp = z * D_800CF1A0[2][2] + temp;
    out = D_800CF1A0[3][2] + temp;

    return out;
}
#ifdef NON_MATCHING
/*
 * PROVENANCE: adapted from JFG's public decomp,
 * src/camera.c:camCopyOrthoMatrix.
 *
 * Plateau: the full flag lattice, eight coherent source/type/indexing
 * variants, and a bounded two-worker permuter batch leave an 84-instruction
 * configured candidate against the 83-instruction target, with 59 positional
 * words different from first mismatch +0x5C. IDO emits one extra address
 * materialization for the third peeled coefficient; the likely blocker is
 * original same-TU data-definition knowledge versus this extern array.
 * The permuter's base score was 135 and it found no improvement.
 */
void func_80024978(MtxF matrix) {
    s32 i;
    s32 width;
    s32 height;

    viGetCurrentSize(&width, &height);
    for (i = 0; i < 15; i++) {
        ((f32 *) matrix)[i] = D_80079F4C[i] * D_80079F48;
    }
    matrix[3][3] = (u32) width >> 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80024978.s")
#endif
/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camStartShake. */
void camStartShake(s32 camNo, f32 attack, f32 sustain, f32 decay,
                   s32 magnitude) {
    CameraShake *shake;

    if ((camNo >= 0) && (camNo < 6)) {
        shake = &D_800CEC18[camNo];
        shake->attackEnd = (s16) (s32) (attack * 60.0f);
        shake->sustainEnd =
            (s16) (shake->attackEnd + (s32) (sustain * 60.0f));
        shake->totalEnd =
            (s16) (shake->sustainEnd + (s32) (decay * 60.0f));
        shake->timer = 0;
        shake->magnitude = magnitude;
    }
}
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
#ifdef NON_MATCHING
/*
 * PROVENANCE: role from JFG's public decomp, src/camera.c:camScreenShake;
 * body reconstructed from Mickey-only evidence.
 *
 * Plateau: after the full flag lattice, ten coherent source/lifetime
 * spellings and a bounded two-worker permuter batch, the closest configured
 * candidate has the exact 296-byte size and differs in 15 positional words
 * from first mismatch +0x60. IDO assigns the long-lived $f20 register to the
 * Z delta instead of the target's X delta, cascading through the arithmetic
 * temporaries; the permuter's best score is 125, not zero.
 */
void func_80024BA0(f32 x, f32 y, f32 z, f32 radius, f32 magnitude) {
    Camera *cam;
    f32 dx;
    f32 distance;
    f32 dz;
    f32 dy;
    f32 attack;
    f32 sustain;
    s32 i;

    i = 0;
    if (D_800CEC60 >= 0) {
        sustain = D_80081A40;
        attack = D_80081A44;
        do {
            cam = &D_800CEA20[i];
            dx = x - cam->transform.x;
            dy = y - cam->transform.y;
            distance = z - cam->transform.z;
            dz = distance;
            dx = dx * dx;
            dy = dy * dy;
            dz = dz * dz;
            distance = sqrtf((dx + dy) + dz);
            if (distance < radius) {
                camStartShake(i, attack, sustain, attack,
                              (s32) (((radius - distance) * magnitude) /
                                     radius));
            }
            i++;
        } while (i <= D_800CEC60);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80024BA0.s")
#endif

/* PROVENANCE: adapted from JFG's public decomp, src/camera.c:camSetZoom. */
void camSetZoom(s32 camNo, f32 zoom) {
    if ((camNo >= 0) && (camNo < 6)) {
        D_80079FA0[camNo] = 1;
        D_80079FB0[camNo] = zoom;
    }
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/camera/func_80024D00.s")
/* Mickey-only fixed-distance camera-transform reconstruction. */
void func_80024ED8(CameraTransform *source, s32 unused, Camera *dest) {
    f32 targetX;
    f32 targetY;
    f32 targetZ;
    f32 deltaX;
    f32 deltaY;
    f32 deltaZ;
    f32 distance;

    if (D_800CEC60 == 0) {
        distance = 100.0f;
    } else {
        distance = 150.0f;
    }
    targetX = source->x - (func_8002A8C0(source->yRotation) * distance);
    targetY = source->y + (distance * D_80081A48);
    targetZ = source->z - (func_8002A8BC(source->yRotation) * distance);
    dest->transform.x = targetX;
    dest->unk18 = targetX;
    dest->unk1C = targetY;
    dest->transform.y = targetY;
    dest->unk20 = targetZ;
    dest->transform.z = targetZ;
    deltaX = targetX - source->x;
    deltaY = (targetY - source->y) - 20.0f;
    deltaZ = targetZ - source->z;
    dest->transform.yRotation = Arctanf(deltaZ, deltaX) + 0x4000;
    dest->transform.xRotation =
        Arctanf(deltaY, sqrtf((deltaX * deltaX) + (deltaZ * deltaZ)));
    dest->transform.zRotation = 0;
}
