/*
 * Video-interface and framebuffer management -- ROM 0x34180-0x34E60
 * (VRAM 0x80033580-0x80034260).
 *
 * The translation-unit boundary follows Jet Force Gemini's public gameVi.c:
 * Mickey has the same complete, ordered 23-function sequence from viInit to
 * fb_memcpy. The preceding function is the separately measured
 * trapDanglingJump TU; the next function has the structure and call surface of
 * texInitTextures, the first function in JFG's following textures.c TU.
 *
 * PROVENANCE -- the translation-unit name and function identifications used
 * while reconstructing this file come from JFG's public decompilation,
 * src/gameVi.c and src/gameVi.h. Mickey's own bytes decide every boundary,
 * type, body and matching verdict. Unmatched functions retain their Mickey
 * func_ names below; only evidence-backed names are adopted.
 *
 * Flags: -O2 -mips2 -32, the resident game-code group.
 */

#include "game/gameVi.h"

extern s32 *D_8007A690[3];
extern f32 D_8007A69C;
extern f32 D_8007A6A0;
extern u8 D_8007A1A0;
extern s32 *D_800D2FA8;
extern s8 D_800D2F95;
extern s8 D_800D2F96;
extern s8 D_800D2F97;
extern s8 D_800D2F98;
extern s8 D_800D2F9A;
extern u8 D_800D2F9B;
extern u8 D_800D2F9C;
extern u8 D_800D2F9D;
extern s32 D_800D2FBC;

extern void func_80033B24(void);
extern void osWritebackDCacheAll(void);

#pragma GLOBAL_ASM("asm/nonmatchings/main/gameVi/func_80033580.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gameVi/func_800336A8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gameVi/func_800339B4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gameVi/func_80033A7C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gameVi/func_80033AD4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gameVi/func_80033B24.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gameVi/func_80033CBC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gameVi/func_80033D04.s")
/*
 * PROVENANCE: adapted from JFG's public decomp, src/gameVi.c, where this is
 * viGetScaleXY. Mickey has no same-address caller evidence and the exact
 * skeleton is below the tier-A uniqueness threshold, so the name is not
 * adopted.
 */
void func_80033D58(f32 *hScale, f32 *vScale) {
    *hScale = D_8007A69C;
    *vScale = D_8007A6A0;
}

/* PROVENANCE: adapted from JFG's public decomp, src/gameVi.c. */
void viFrameRateReset(void) {
    D_800D2F9D = 1;
    D_800D2F9B = 0;
    D_800D2F9C = 2;
    D_800D2FBC = 0;
}

#pragma GLOBAL_ASM("asm/nonmatchings/main/gameVi/func_80033DA0.s")
/* PROVENANCE: adapted from JFG's public decomp, src/gameVi.c. */
s32 viGetVideoMode(void) {
    return D_800D2F98 & 3;
}

/* PROVENANCE: adapted from JFG's public decomp, src/gameVi.c. */
s8 viGetWideAdjust(void) {
    return D_800D2F9A;
}

/* PROVENANCE: adapted from JFG's public decomp, src/gameVi.c. */
void viSetWideAdjust(s32 offset) {
    if (offset < -30) {
        offset = -30;
    }
    if (offset > 30) {
        offset = 30;
    }
    D_800D2F9A = offset;
    func_80033B24();
}

/* PROVENANCE: adapted from JFG's public decomp, src/gameVi.c. */
void viSetTrippleBuffer(s32 resolutionIndex) {
    D_800D2F96 = resolutionIndex & 1;
}

/*
 * PROVENANCE: adapted from JFG's public decomp, src/gameVi.c, where this is
 * viGetTrippleBuffer. No same-address Mickey caller pins that public name, so
 * the canonical function retains its address label.
 */
s8 func_80033FB8(void) {
    return D_800D2F95;
}

/* PROVENANCE: adapted from JFG's public decomp, src/gameVi.c. */
s32 viChangeBuffers(void) {
    return D_800D2F95 != D_800D2F96;
}

/*
 * PROVENANCE: adapted from JFG's public decomp, src/gameVi.c, where this is
 * viNoClear. No same-address Mickey caller pins that public name, so the
 * canonical function retains its address label.
 */
void func_80033FE0(void) {
    D_800D2F97 = 0;
}

/* PROVENANCE: adapted from JFG's public decomp, src/gameVi.c. */
s32 viDisplayingScreen0(void) {
    if (D_8007A690[0] == D_800D2FA8) {
        return 1;
    }
    return 0;
}

/* PROVENANCE: adapted from JFG's public decomp, src/gameVi.c. */
void func_80034018(u8 *src, s32 length) {
    s32 *dest;
    s32 fourByteLength;

    dest = (s32 *) src;
    fourByteLength = length >> 2;
    if (D_8007A1A0 != 0) {
        while (fourByteLength--) {
            *dest++ = 0;
        }
    } else {
        while (fourByteLength--) {
            *dest++ = -1;
        }
    }
    osWritebackDCacheAll();
}

#pragma GLOBAL_ASM("asm/nonmatchings/main/gameVi/func_80034094.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gameVi/func_80034150.s")

/* PROVENANCE: adapted from JFG's public decomp, src/gameVi.c. */
void fb_memcpy(u8 *src, u8 *dest, s32 len) {
    while (len--) {
        *dest++ = *src++;
    }
}
