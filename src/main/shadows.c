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

/* Shadow buffer state occupies one contiguous compiler-owned .data input
 * section. Keep the retail labels at their measured offsets for all users;
 * the terminal halfword pair is part of IDO's measured 0x50-byte section. */
u8 *D_80079410[1] = { 0 };
u8 *D_80079414[3] = { 0 };
u8 *D_80079420[1] = { 0 };
u8 *D_80079424[3] = { 0 };
u8 *D_80079430[1] = { 0 };
u8 *D_80079434[3] = { 0 };
u8 *D_80079440 = 0;
u8 *D_80079444 = 0;
u8 *D_80079448 = 0;
s32 D_8007944C = 0;
s32 D_80079450 = 0;
s32 D_80079454 = 0;
s32 D_80079458 = 0;
u16 D_8007945C[2] = { 0, 0x4000 };
extern s32 D_800CB278;
extern s32 D_800CB27C;
extern s32 D_800CB280;
extern s32 D_800CB268;
extern s32 D_800CB26C;
extern s32 D_800C9D40;
extern f32 func_8002A8BC(s16 angle);
extern f32 func_8002A8C0(s16 angle);
extern s32 D_800CAF58;
extern u8 D_800CAF60[];
extern u8 D_800C9D48[];
extern u8 D_800C9F58[];
extern s32 D_800C9F48[];
extern f32 D_800CB260;
extern f32 D_800CB270;
extern f32 D_800CB274;
extern s32 D_800CB284;
extern s32 D_800CB288;
extern void *func_8002B280(s32 size, s32 tag);
extern void mmFree(void *ptr);
extern s32 getXZCompareMask(void *grid, s32 xMin, s32 zMin, s32 xMax, s32 zMax);
extern s32 mathXZInTri(s32 x, s32 z, void *a, void *b, void *c);
extern f32 D_80079464[];
extern f32 D_800CB28C;

typedef struct ShadowQueryVolume {
    u8 pad0[0x6C];
    s16 minY6C;
    s16 maxY6E;
} ShadowQueryVolume;

typedef struct ShadowQuery {
    u8 pad0[0xC];
    f32 x0C;
    f32 y10;
    f32 z14;
    u8 pad18[0x16];
    s16 sector2E;
    u8 pad30[0x10];
    ShadowQueryVolume *volume40;
    u8 pad44[0xC];
    f32 *value50;
} ShadowQuery;

typedef struct ShadowWorld {
    u8 pad0[4];
    void *sectors4;
    u32 *grid8;
} ShadowWorld;

typedef struct ShadowSector {
    u8 *vertices0;
    u8 *triangles4;
    u8 pad8[4];
    struct ShadowBlock *blocksC;
    u32 *masks10;
    u8 pad14[0x10];
    s16 blockCount24;
} ShadowSector;

typedef struct ShadowBlock {
    u8 pad0[6];
    u8 vertexBase6;
    u8 pad7;
    s16 firstVertex8;
    u8 padA[2];
    u32 flagsC;
    u8 pad10[8];
    s16 lastVertex18;
} ShadowBlock;

typedef struct ShadowTriangle {
    u8 pad0;
    u8 vertex1;
    u8 vertex2;
    u8 vertex3;
} ShadowTriangle;
extern s32 func_80017660(void *arg0, s32 arg1, void *arg2, s32 arg3, s32 arg4);
extern void func_80018544(void *arg0, void *arg1);
extern s32 shadowBoxPolyOverlap(f32 arg0, f32 arg1, s32 arg2, s16 arg3,
                                s32 arg4, s32 arg5, s32 arg6, void *arg7);

/* PROVENANCE: adapted from JFG's public asm/nonmatchings/shadows/shadowInitBuffers.s; Mickey globals are authoritative.
 * The C body emits all 75 linked instruction words and the owning 0x50-byte
 * .data section exactly. Its sentinel pair still binds D_80079434 + 0xC where
 * the target relocation metadata names D_80079440, so relocation identity is
 * not exact. */
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
/*
 * PROVENANCE: organized from the public JFG shadow polygon pipeline and
 * Mickey's own m2c control flow; all field offsets and buffer limits remain
 * Mickey-only evidence.
 */
#ifdef NON_MATCHING
/* Workbench verdict: structure-mismatch, 337 differing words; first mismatch is at +0x0. */
/* Target is 328 instructions/frame -320; candidate is 344 instructions/frame -280. */
/* Remaining gap is structural: pointer/stack scheduling and frame shape; not permuter-ready. */
void func_80017140(void *arg0, s32 arg1, void *arg2, s32 arg3) {
    u8 polygon[0x30];
    u8 *var_a3;
    u8 *temp_a3;
    u8 *temp_t1;
    u8 *temp_t2;
    u8 *temp_v0;
    u8 *var_v0;
    u8 *var_v1_2;
    u8 *var_v1_3;
    u8 *var_v1_4;
    u8 *trackBase;
    u8 *vertexBase;
    f32 pointHeight;
    s16 temp_a1;
    s16 temp_v0_3;
    s16 temp_v1;
    s16 var_a0_3;
    s16 var_a1;
    s16 var_a1_2;
    s16 var_a2;
    s16 var_ra;
    s16 var_t0;
    s32 temp_s1;
    s32 temp_t9;
    s32 var_a0;
    s32 var_a0_2;
    s32 var_a2_2;
    s32 var_lo;
    s32 var_t0_2;
    s32 var_t1;
    s32 var_v1;
    s8 temp_v0_4;
    u32 temp_a0;
    u32 temp_v0_2;
    u32 *maskBase;

    var_t0 = *(s16 *) ((u8 *) arg2 + 0x24);
    var_t1 = 0;
    if (var_t0 > 0) {
        s32 sp7C = 0;
        do {
            temp_v0 = *(u8 **) ((u8 *) arg2 + 0xC) + sp7C;
            temp_a0 = *(u32 *) (temp_v0 + 0xC);
            trackBase = *(u8 **) ((u8 *) arg2 + 0x0);
            maskBase = *(u32 **) ((u8 *) arg2 + 0x10);
            vertexBase = *(u8 **) ((u8 *) arg2 + 0x1C);
            if (!(temp_a0 & 0x08013880)) {
                var_ra = *(s16 *) (temp_v0 + 0x8);
                temp_a1 = *(s16 *) (temp_v0 + 0x18);
                temp_s1 = (s32) (trackBase + (*(s16 *) (temp_v0 + 0x6) * 0xA));
                if (var_ra < temp_a1) {
                    var_v1 = var_ra * 8;
                    var_a0 = var_ra * 4;
                    do {
                        temp_t9 = *(u32 *) (*(u8 **) ((u8 *) arg2 + 0x18) + var_v1) * 4;
                        temp_v0_2 = (*(u32 *) (maskBase + var_a0)) & arg3;
                        if ((temp_v0_2 & 0xFFFF) &&
                            ((temp_v0_2 >> 0x10) != 0) &&
                            (*(f32 *) (vertexBase + (temp_t9 * 4) + 0x4) > 0.5f)) {
                            var_a0_2 = 1;
                            temp_a3 = *(u8 **) ((u8 *) arg2 + 0x4) + (var_ra * 0x10);
                            var_v1_2 = temp_a3 + 1;
                            var_a1 = *(s8 *) (temp_s1 + (*(u8 *) (temp_a3 + 1) * 0xA) + 0x2);
                            var_a2 = var_a1;
                            do {
                                var_a0_2 += 1;
                                temp_v0_3 = *(s8 *)
                                    (temp_s1 + (*(u8 *) (var_v1_2 + 1) * 0xA) + 0x2);
                                if (temp_v0_3 < var_a1) {
                                    var_a1 = temp_v0_3;
                                } else if (var_a2 < temp_v0_3) {
                                    var_a2 = temp_v0_3;
                                }
                                var_v1_2 += 1;
                            } while (var_a0_2 < 3);
                            if (*(s16 *) ((u8 *) arg0 + 0x18) >= var_a1) {
                                var_v1_3 = temp_a3;
                                if (var_a2 >= *(s16 *) ((u8 *) arg0 + 0x16)) {
                                    var_v0 = polygon + 0x10;
                                    var_lo = *(u8 *) (var_v1_3 + 1) * 0xA;
                                    while (var_v0 != polygon + 0x30) {
                                        var_v0 += 0x10;
                                        var_v1_3 += 1;
                                        *(f32 *) (var_v0 - 0x20) =
                                            (f32) *(s16 *) (temp_s1 + var_lo);
                                        *(s32 *) (var_v0 - 0x12) = -1;
                                        *(f32 *) (var_v0 - 0x18) =
                                            (f32) *(s16 *)
                                                (temp_s1 + (*(u8 *) (var_v1_3 + 0x0) * 0xA) + 0x4);
                                        var_lo = *(u8 *) (var_v1_3 + 1) * 0xA;
                                    }
                                    *(f32 *) (var_v0 - 0x10) =
                                        (f32) *(s16 *) (temp_s1 + var_lo);
                                    *(s32 *) (var_v0 - 0x2) = -1;
                                    *(f32 *) (var_v0 - 0x8) =
                                        (f32) *(s16 *)
                                            (temp_s1 + (*(u8 *) (var_v1_3 + 1) * 0xA) + 0x4);
                                    if (shadowBoxPolyOverlap(
                                            *(f32 *) ((u8 *) arg0 + 0x40),
                                            *(f32 *) ((u8 *) arg0 + 0x44),
                                            var_a0_2, var_a1,
                                            *(s32 *) ((u8 *) arg0 + 0x48),
                                            *(s32 *) ((u8 *) arg0 + 0x4C), 3,
                                            polygon) != 0) {
                                        *(u8 **) ((u8 *) arg0 + 0x4) =
                                            vertexBase + (temp_t9 * 4);
                                        if (*(f32 *) ((u8 *) arg0 + 0x24) > 0.0f) {
                                            func_80018544(arg0, polygon);
                                        }
                                        temp_v0_4 = func_80017660(arg0, 3, polygon, 4, arg1);
                                        if (temp_v0_4 >= 3) {
                                            temp_t2 = D_800CAF60 + (D_800CAF58 * 0xC);
                                            *(u8 *) (temp_t2 + 1) = 0;
                                            var_t0_2 = 0;
                                            if (temp_v0_4 > 0) {
                                                var_a3 = polygon;
                                                do {
                                                    temp_v1 = *(s16 *) (var_a3 + 0xE);
                                                    var_a1_2 = -1;
                                                    var_a0_3 = 0;
                                                    if (temp_v1 < 0) {
                                                        var_a2_2 = D_800C9D40;
                                                        temp_t1 = temp_t2 + var_t0_2;
                                                        if (var_a2_2 > 0) {
                                                            var_v1_4 = D_800C9D48;
loop_27:
                                                            if ((*(f32 *) (var_v1_4 + 0x0) ==
                                                                 *(f32 *) (var_a3 + 0x0)) &&
                                                                (*(f32 *) (var_v1_4 + 0x8) ==
                                                                 *(f32 *) (var_a3 + 0x8))) {
                                                                var_a1_2 = var_a0_3;
                                                            }
                                                            var_a0_3 += 1;
                                                            var_v1_4 += 0x10;
                                                            if ((var_a0_3 < var_a2_2) &&
                                                                (var_a1_2 == -1)) {
                                                                goto loop_27;
                                                            }
                                                        }
                                                        if (var_a1_2 == -1) {
                                                            if (var_a2_2 >= 0x20) {
                                                                D_800C9D40 = 0x1F;
                                                                var_a2_2 = 0x1F;
                                                            }
                                                            var_v1_4 = D_800C9D48 + (var_a2_2 * 0x10);
                                                            *(f32 *) (var_v1_4 + 0x0) = *(f32 *) (var_a3 + 0x0);
                                                            *(f32 *) (var_v1_4 + 0x8) = *(f32 *) (var_a3 + 0x8);
                                                            D_800C9D40 = var_a2_2 + 1;
                                                            *(s8 *) (temp_t1 + 0x2) = var_a2_2;
                                                            *(s32 *) (var_v1_4 + 0xC) =
                                                                *(s32 *) ((u8 *) arg0 + 0x4);
                                                        } else {
                                                            *(s8 *) (temp_t1 + 0x2) = var_a1_2;
                                                        }
                                                    } else {
                                                        *(s8 *) (temp_t2 + var_t0_2 + 0x2) = temp_v1;
                                                        *(u8 *) (temp_t2 + 1) |= (1 << var_t0_2);
                                                    }
                                                    var_t0_2 += 1;
                                                    var_a3 += 0x10;
                                                } while (var_t0_2 != temp_v0_4);
                                            }
                                            *(u8 *) (temp_t2 + 0x0) = temp_v0_4;
                                            *(s16 *) (temp_t2 + 0xA) = *(s32 *) ((u8 *) temp_v0 + 0x94);
                                            D_800CAF58 += 1;
                                            D_800CB26C = 0;
                                            D_800CB268 = *(s32 *) ((u8 *) temp_v0 + 0x94);
                                        }
                                    }
                                }
                            }
                        }
                        var_ra += 1;
                        var_v1 += 8;
                        var_a0 += 4;
                    } while (var_ra < temp_a1);
                    var_t0 = *(s16 *) ((u8 *) arg2 + 0x24);
                }
            }
            var_t1 += 1;
            sp7C += 0x10;
        } while (var_t1 < var_t0);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/shadows/func_80017140.s")
#endif
/*
 * PROVENANCE: the clipping/intersection organization follows JFG's public
 * shadow pipeline; Mickey's target assembly and resident buffers determine
 * every field binding and limit used here.
 */
#ifdef NON_MATCHING
/* Workbench verdict: structure-mismatch, 368 differing words; first mismatch is at +0x0. */
/* Target is 347 instructions/frame -344; candidate is 370 instructions/frame -448. */
/* Remaining gap is structural: local-frame and clipping-loop shape; not permuter-ready. */
s32 func_80017660(void *arg0, s32 arg1, void *arg2, s32 arg3, s32 arg4) {
    u8 clipped[0x80];
    u8 *temp_a0;
    u8 *temp_a1;
    u8 *temp_a2;
    u8 *temp_a2_2;
    u8 *temp_a3;
    u8 *temp_t1;
    u8 *temp_v0;
    u8 *var_a0_2;
    u8 *var_a0_3;
    u8 *var_s0;
    u8 *var_s2;
    u8 *var_s6;
    u8 *var_t1;
    u8 *var_t1_2;
    u8 *var_t1_3;
    u8 *var_v0_2;
    u8 *arg4Bytes;
    f32 temp_f0;
    f32 temp_f0_2;
    f32 temp_f0_3;
    f32 temp_f12;
    f32 temp_f12_2;
    f32 temp_f14;
    f32 temp_f14_2;
    f32 temp_f16;
    f32 temp_f20;
    f32 temp_f22;
    f32 temp_f24;
    f32 temp_f28;
    f32 temp_f28_2;
    f32 temp_f2;
    f32 temp_f2_2;
    f32 temp_f30;
    f32 temp_f30_2;
    f32 temp_f30_3;
    f32 var_f24;
    s16 var_a0;
    s16 var_t2;
    s16 var_t7;
    s16 var_t9;
    s32 *temp_t3;
    s32 temp_a3_2;
    s32 temp_s7;
    s32 temp_t4;
    s32 temp_v1;
    s32 temp_v1_2;
    s32 var_s1;
    s32 var_t0;
    s32 var_t5;
    s32 var_v0;
    s32 var_v0_3;
    s32 var_v1;
    s32 var_v1_2;
    s32 var_v1_3;

    var_s2 = (u8 *) arg2;
    var_s0 = clipped;
    var_s1 = arg1;
    var_t5 = 0;
    arg4Bytes = (u8 *) arg4;
    if ((arg3 > 0) && (arg1 >= 3)) {
        var_s6 = arg4Bytes;
loop_3:
        temp_s7 = var_t5 + 1;
        var_v1 = temp_s7;
        var_v0 = 0;
        var_t0 = 0;
        if (temp_s7 >= arg3) {
            var_v1 = 0;
        }
        temp_f14 = *(f32 *) (var_s6 + 0x0);
        temp_a0 = arg4Bytes + (var_v1 * 8);
        temp_f12 = *(f32 *) (temp_a0 + 0x0);
        temp_f0 = *(f32 *) (temp_a0 + 0x4);
        temp_f2 = *(f32 *) (var_s6 + 0x4);
        var_t1 = var_s2;
        temp_f20 = temp_f0 - temp_f2;
        temp_f22 = -(temp_f12 - temp_f14);
        if (temp_f14 < temp_f12) {
            var_f24 = (temp_f2 * temp_f22) + (temp_f20 * temp_f14);
        } else {
            var_f24 = (temp_f0 * temp_f22) + (temp_f20 * temp_f12);
        }
        temp_f24 = -var_f24;
        if (var_s1 > 0) {
loop_9:
            temp_t4 = var_v0 + 1;
            var_v1_2 = temp_t4;
            if (temp_t4 >= var_s1) {
                var_v1_2 = 0;
            }
            temp_f0_2 = *(f32 *) (var_t1 + 0x8);
            temp_f2_2 = *(f32 *) (var_t1 + 0x0);
            temp_a3 = var_s2 + (var_v1_2 * 0x10);
            temp_f28 = *(f32 *) (temp_a3 + 0x8);
            temp_f12_2 = *(f32 *) (temp_a3 + 0x0);
            temp_f14_2 = (temp_f0_2 * temp_f22) +
                         (temp_f20 * temp_f2_2) + temp_f24;
            temp_f16 = (temp_f28 * temp_f22) +
                       (temp_f20 * temp_f12_2) + temp_f24;
            if (((temp_f14_2 >= 0.0f) && (temp_f16 < 0.0f)) ||
                ((temp_f14_2 < 0.0f) && (temp_f16 >= 0.0f))) {
                temp_t3 = (s32 *) (D_800C9F48 + (var_t5 * 4));
                var_v1_3 = *temp_t3;
                var_a0 = var_t5 << 5;
                var_t2 = -1;
                var_v0_2 = D_800C9F58 + (var_a0 << 5);
                temp_a2 = var_s0 + (var_t0 * 0x10);
                if (var_v1_3 > 0) {
loop_16:
                    temp_f30 = *(f32 *) (var_v0_2 + 0x10);
                    if (((temp_f30 == temp_f2_2) &&
                         (*(f32 *) (var_v0_2 + 0x14) == temp_f0_2) &&
                         (*(f32 *) (var_v0_2 + 0x18) == temp_f12_2) &&
                         (*(f32 *) (var_v0_2 + 0x1C) == temp_f28)) ||
                        ((var_v1_3 -= 1,
                          (temp_f30 == temp_f12_2)) &&
                         (*(f32 *) (var_v0_2 + 0x14) == temp_f28) &&
                         (*(f32 *) (var_v0_2 + 0x18) == temp_f2_2) &&
                         (*(f32 *) (var_v0_2 + 0x1C) == temp_f0_2))) {
                        var_t2 = var_a0;
                    } else {
                        var_v0_2 += 0x20;
                        var_a0 += 1;
                        if (var_v1_3 > 0) {
                            goto loop_16;
                        }
                    }
                }
                if (var_t2 >= 0) {
                    *(s16 *) (temp_a2 + 0xE) = var_t2;
                    var_t0 += 1;
                    *(f32 *) (temp_a2 + 0x0) = *(f32 *) (var_v0_2 + 0x0);
                    *(f32 *) (temp_a2 + 0x8) = *(f32 *) (var_v0_2 + 0x8);
                    if (var_t0 >= 8) {
                        return 0;
                    }
                    goto block_32;
                }
                var_t0 += 1;
                temp_f28_2 = temp_f14_2 / (temp_f14_2 - temp_f16);
                *(f32 *) (temp_a2 + 0x0) =
                    (((temp_f12_2 - temp_f2_2) * temp_f28_2) + temp_f2_2);
                temp_f0_3 = *(f32 *) (var_t1 + 0x8);
                *(f32 *) (temp_a2 + 0x8) =
                    (((*(f32 *) (temp_a3 + 0x8) - temp_f0_3) * temp_f28_2) +
                     temp_f0_3);
                *(f32 *) (var_v0_2 + 0x10) = *(f32 *) (var_t1 + 0x0);
                *(f32 *) (var_v0_2 + 0x14) = *(f32 *) (var_t1 + 0x8);
                *(f32 *) (var_v0_2 + 0x18) = *(f32 *) (temp_a3 + 0x0);
                *(f32 *) (var_v0_2 + 0x1C) = *(f32 *) (temp_a3 + 0x8);
                *(f32 *) (var_v0_2 + 0x0) = *(f32 *) (temp_a2 + 0x0);
                *(f32 *) (var_v0_2 + 0x8) = *(f32 *) (temp_a2 + 0x8);
                *(s32 *) (var_v0_2 + 0xC) = *(s32 *) ((u8 *) arg0 + 0x4);
                *(s16 *) (temp_a2 + 0xE) = var_a0;
                if (var_t0 >= 8) {
                    return 0;
                }
                *temp_t3 += 1;
                goto block_32;
            }
block_32:
            var_v0 = temp_t4;
            if (temp_f16 <= 0.0f) {
                temp_a2_2 = var_s0 + (var_t0 * 0x10);
                *(s16 *) (temp_a2_2 + 0xE) = *(s16 *) (temp_a3 + 0xE);
                var_t0 += 1;
                *(f32 *) (temp_a2_2 + 0x0) = *(f32 *) (temp_a3 + 0x0);
                *(f32 *) (temp_a2_2 + 0x8) = *(f32 *) (temp_a3 + 0x8);
                if (var_t0 >= 8) {
                    return 0;
                }
            }
            var_t1 += 0x10;
            if (temp_t4 == var_s1) {
                goto block_36;
            }
            goto loop_9;
        }
block_36:
        temp_v0 = var_s2;
        var_s2 = var_s0;
        var_s1 = var_t0;
        var_t5 = temp_s7;
        var_s6 += 8;
        var_s0 = temp_v0;
        if ((temp_s7 >= arg3) || (var_t0 < 3)) {
            goto block_38;
        }
        goto loop_3;
    }
block_38:
    if (var_s1 >= 3) {
        if (var_s2 != (u8 *) arg2) {
            var_v0_3 = 0;
            if (var_s1 > 0) {
                for (temp_a3_2 = var_s1; temp_a3_2 > 0; temp_a3_2--) {
                    temp_a1 = (u8 *) arg2 + (var_v0_3 * 0x10);
                    temp_a2 = var_s2 + (var_v0_3 * 0x10);
                    *(f32 *) (temp_a1 + 0x0) = *(f32 *) (temp_a2 + 0x0);
                    *(f32 *) (temp_a1 + 0x8) = *(f32 *) (temp_a2 + 0x8);
                    *(s16 *) (temp_a1 + 0xE) = *(s16 *) (temp_a2 + 0xE);
                    *(s32 *) (temp_a1 + 0x10) = *(s32 *) (temp_a2 + 0x10);
                    var_v0_3 += 1;
                }
            }
        }
    } else {
        var_s1 = 0;
    }
    return var_s1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/shadows/func_80017660.s")
#endif
/*
 * PROVENANCE: adapted from the public Diddy Kong Racing/JFG shadow-buffer
 * and projected-triangle organization; Mickey's target bytes, globals, and
 * resident buffer layouts determine the field bindings below.
 */
#ifdef NON_MATCHING
/* Workbench verdict: structure-mismatch, 312 differing words; first mismatch is at +0x0. */
/* Target is 314 instructions/frame -264; candidate is 326 instructions/frame -280. */
/* Remaining gap is structural: prologue/constant setup and pointer/register scheduling; not permuter-ready. */
s32 func_80017BCC(void *arg0, void *arg1, void *arg2) {
    u32 projected[3];
    u8 *var_a0;
    u8 *var_a2;
    u8 *var_a3;
    u8 *var_s6;
    u8 *var_t4;
    u8 *var_v0;
    u8 *var_v0_2;
    u8 *temp_v0;
    u8 *source;
    volatile f32 spA8;
    f32 temp_f0;
    f32 temp_f12;
    f32 temp_f12_2;
    f32 temp_f2;
    f32 temp_f2_2;
    f32 temp_f2_3;
    f32 var_f0;
    f32 var_f0_2;
    f32 var_f12;
    f32 var_f14;
    f32 var_f16;
    f32 var_f18;
    f32 var_f22;
    f32 var_f24;
    f32 var_f26;
    f32 var_f28;
    s32 var_fp;
    s32 var_s4;
    s32 var_s7;
    s32 var_t2;
    s32 var_t3;
    s32 var_v1;
    s32 var_v1_2;
    s8 var_a0_2;
    s8 var_a1_2;
    s8 var_t5;
    u8 var_a1;
    u8 vertexCount;

    if ((*(u8 *) ((u8 *) arg2 + 0x10) & 0x10) != 0) {
        var_f16 = 1.0f;
        var_f14 = 0.0f;
    } else {
        if (arg1 != NULL) {
            spA8 = func_8002A8C0(*(s16 *) ((u8 *) arg1 + 0x0));
            var_f0 = func_8002A8BC(*(s16 *) ((u8 *) arg1 + 0x0));
            var_f14 = spA8;
        } else {
            spA8 = func_8002A8C0(*(s16 *) ((u8 *) arg0 + 0x14));
            var_f0 = func_8002A8BC(*(s16 *) ((u8 *) arg0 + 0x14));
            var_f14 = spA8;
        }
        var_f16 = var_f0;
    }
    temp_v0 = *(u8 **) ((u8 *) arg0 + 0x0);
    var_f26 = *(f32 *) ((u8 *) arg0 + 0x34);
    temp_f12 = *(f32 *) ((u8 *) arg0 + 0x24);
    var_f28 = *(f32 *) ((u8 *) arg0 + 0x38);
    var_f18 = 255.0f;
    var_t5 = 0x19;
    var_t4 = D_800CAF60;
    var_s7 = 0;
    var_f22 = (f32) (*(u16 *) (temp_v0 + 0x6) * 0x10) / var_f26;
    var_f24 = (f32) (*(u16 *) (temp_v0 + 0x8) << 5) /
              (var_f28 + *(f32 *) ((u8 *) arg0 + 0x3C));
    if (temp_f12 > 0.0f) {
        temp_f2 = *(f32 *) ((u8 *) arg0 + 0xC) -
                  *(f32 *) ((u8 *) arg0 + 0x20);
        if (temp_f12 < temp_f2) {
            var_f18 = 255.0f *
                      (1.0f - ((temp_f2 - temp_f12) /
                               *(f32 *) ((u8 *) arg0 + 0x28)));
            if (var_f18 < 0.0f) {
                var_f18 = 0.0f;
            }
        }
        if (temp_f2 > 0.0f) {
            temp_f0 = (temp_f2 / 200.0f) + 1.0f;
            var_f22 *= temp_f0;
            var_f26 /= temp_f0;
            var_f24 *= temp_f0;
            var_f28 /= temp_f0;
        }
    }
    var_s4 = (s32) (var_f18 * D_800CB260);
    if (arg1 != NULL) {
        var_s4 = (s32) (*(s16 *) ((u8 *) arg1 + 0x4) * var_s4) >> 8;
    }
    var_f18 = D_800CB270;
    var_f0 = D_800CB274;
    var_t2 = D_8007944C;
    var_t3 = D_80079450;
    var_a3 = D_80079444 + (var_t3 * 0x10);
    var_fp = D_80079454;
    var_a2 = D_80079440 + (var_t2 * 0xA);
    var_s6 = D_80079448 + (var_fp * 8);
    if (D_800CAF58 > 0) {
loop_16:
        var_a0 = var_t4;
        if ((*(u8 *) (var_t4 + 0x0) + var_t5) >= 0x18) {
            *(s16 *) (var_s6 + 0x6) = var_t2;
            *(s16 *) (var_s6 + 0x4) = var_t3;
            var_s6 += 8;
            var_fp += 1;
            var_t5 = 0;
            *(u32 *) (var_s6 - 0x8) = *(u32 *) ((u8 *) arg0 + 0x0);
        }
        if (var_fp >= D_800CB280) {
            return 0;
        }
        var_a1 = *(u8 *) (var_t4 + 0x1);
        var_v1 = 0;
        vertexCount = *(u8 *) (var_t4 + 0x0);
        if ((s32) vertexCount > 0) {
loop_21:
            if (var_a1 & 1) {
                var_v0 = D_800C9F58 + (*(u8 *) (var_a0 + 0x2) << 5);
                var_f0_2 = *(f32 *) (var_v0 + 0x0);
                var_f12 = *(f32 *) (var_v0 + 0x4);
            } else {
                var_v0 = D_800C9D48 + (*(u8 *) (var_a0 + 0x2) * 0x10);
                var_f0_2 = *(f32 *) (var_v0 + 0x0);
                var_f12 = *(f32 *) (var_v0 + 0x4);
            }
            temp_f2_2 = *(f32 *) (var_v0 + 0x8);
            var_a1 = (u8) ((s32) var_a1 >> 1);
            var_t2 += 1;
            var_a2 += 0xA;
            *(s16 *) (var_a2 - 0xA) = (s32) var_f0_2;
            *(s8 *) (var_a2 - 0x4) = 0xFF;
            *(s8 *) (var_a2 - 0x3) = 0xFF;
            *(s8 *) (var_a2 - 0x2) = 0xFF;
            *(s8 *) (var_a2 - 0x1) = (s8) var_s4;
            *(s16 *) (var_a2 - 0x6) = (s32) temp_f2_2;
            *(s16 *) (var_a2 - 0x8) =
                (s32) (*(f32 *) ((u8 *) arg0 + 0x1C) + var_f12);
            if (var_t2 >= D_800CB278) {
                return 0;
            }
            temp_f12_2 = var_f0_2 - var_f18;
            var_a0 += 1;
            temp_f2_3 = temp_f2_2 - var_f0;
            projected[var_v1] =
                ((s32) (((temp_f2_3 * var_f16) +
                         (temp_f12_2 * var_f14) + var_f28) * var_f24) &
                 0xFFFF) |
                ((s32) (var_f22 * (((temp_f12_2 * var_f16) -
                                    (temp_f2_3 * var_f14)) + var_f26)) <<
                 0x10);
            var_v1 += 1;
            if (var_v1 < (s32) *(u8 *) (var_t4 + 0x0)) {
                goto loop_21;
            }
        }
        var_v1_2 = 1;
        if ((*(u8 *) (var_t4 + 0x0) - 1) >= 2) {
            var_a0_2 = var_t5 + 1;
            var_a1_2 = var_a0_2 + 1;
            var_v0_2 = (u8 *) &projected[1];
loop_29:
            *(u8 *) (var_a3 + 0x0) = 0;
            *(u8 *) (var_a3 + 0x1) = var_a0_2;
            *(u8 *) (var_a3 + 0x2) = var_a1_2;
            *(u8 *) (var_a3 + 0x3) = var_t5;
            *(u32 *) (var_a3 + 0x4) = *(u32 *) (var_v0_2 + 0x0);
            var_t3 += 1;
            var_v1_2 += 1;
            *(u32 *) (var_a3 + 0x8) = *(u32 *) (var_v0_2 + 0x4);
            *(u32 *) (var_a3 + 0xC) = projected[0];
            var_a3 += 0x10;
            if (var_t3 >= D_800CB27C) {
                return 0;
            }
            var_v0_2 += 4;
            var_a0_2 += 1;
            var_a1_2 += 1;
            if (var_v1_2 < (*(u8 *) (var_t4 + 0x0) - 1)) {
                goto loop_29;
            }
        }
        var_s7 += 1;
        var_t5 += *(u8 *) (var_t4 + 0x0);
        var_t4 += 0xC;
        if (var_s7 < D_800CAF58) {
            goto loop_16;
        }
    }
    D_8007944C = var_t2;
    D_80079450 = var_t3;
    D_80079454 = var_fp;
    return 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/shadows/func_80017BCC.s")
#endif
/* Workbench verdict: structure-mismatch, 179 differing words, first mismatch +0x0. */
/* Candidate: 204/206 instructions with the exact -0x90 frame; two instruction and relocation-position residuals remain. */
/* Shape status: sector/block loops, three-vertex bounds test, and fade update are preserved, but the candidate is not shape-exact. */
/* PROVENANCE: Mickey's m2c control-flow draft and resident shadow offsets supply this reconstruction; no external body is copied. */
#ifdef NON_MATCHING
void func_800180B4(ShadowQuery *query) {
    ShadowQueryVolume *volume;
    ShadowWorld *world;
    ShadowSector *sector;
    ShadowBlock *block;
    ShadowTriangle *triangle;
    ShadowTriangle *nextTriangle;
    u32 flags;
    u32 maskWord;
    s32 y;
    s32 yMin;
    s32 yMax;
    s32 sectorIndex;
    s32 mask;
    s32 blockOffset;
    s32 blockNumber;
    s32 vertex;
    s32 vertexOffset;
    s32 triangleNumber;
    s16 lowY;
    s16 highY;
    s16 currentY;
    f32 *value;
    f32 oldValue;
    f32 targetValue;
    s32 done;

    volume = query->volume40;
    y = (s32) query->y10;
    yMax = y + volume->maxY6E;
    yMin = y + volume->minY6C;
    done = 0;
    sectorIndex = query->sector2E;
    if (sectorIndex != -1) {
        world = (ShadowWorld *) D_800CB284;
        mask = getXZCompareMask(
            (u8 *) world->grid8 + (sectorIndex * 0xC),
            (s32) (query->x0C - 16.0f),
            (s32) (query->z14 - 16.0f),
            (s32) (query->x0C + 16.0f),
            (s32) (query->z14 + 16.0f));
        blockNumber = 0;
        sector = (ShadowSector *) ((u8 *) world->sectors4 + (sectorIndex << 6));
        blockOffset = 0;
        block = sector->blocksC;
        if (sector->blockCount24 > 0) {
            do {
                flags = block->flagsC;
                if ((flags & 0x08013880) == 0) {
                    vertex = block->firstVertex8;
                    vertexOffset = vertex * 4;
                    if ((vertex < block->lastVertex18) && (done == 0)) {
                        do {
                            maskWord = *(u32 *) ((u8 *) sector->masks10 + vertexOffset);
                            if (((maskWord & mask) != 0) &&
                                ((maskWord >> 16) != 0)) {
                                triangle = (ShadowTriangle *)
                                    ((u8 *) sector->triangles4 +
                                     (vertex * 0x10));
                                nextTriangle = triangle + 1;
                                lowY = *(s16 *) ((u8 *) sector->vertices0 +
                                                 (triangle->vertex1 * 0xA) + 2);
                                highY = lowY;
                                triangleNumber = 1;
                                do {
                                    triangleNumber++;
                                    currentY = *(s16 *)
                                        ((u8 *) sector->vertices0 +
                                         (nextTriangle->vertex1 * 0xA) + 2);
                                    if (currentY < lowY) {
                                        lowY = currentY;
                                    } else if (highY < currentY) {
                                        highY = currentY;
                                    }
                                    nextTriangle++;
                                } while (triangleNumber != 3);
                                if ((highY >= yMin) && (yMax >= lowY) &&
                                    (mathXZInTri((s32) query->x0C,
                                                 (s32) query->z14,
                                                 (u8 *) sector->vertices0 +
                                                     (triangle->vertex1 * 0xA),
                                                 (u8 *) sector->vertices0 +
                                                     (triangle->vertex2 * 0xA),
                                                 (u8 *) sector->vertices0 +
                                                     (triangle->vertex3 * 0xA)) != 0)) {
                                    value = query->value50;
                                    oldValue = *value;
                                    done = 1;
                                    targetValue =
                                        (1.0f - D_80079464[(flags >> 24) & 7]) -
                                        oldValue;
                                    *value = oldValue + (targetValue * D_800CB28C);
                                }
                            }
                            vertex++;
                            vertexOffset += 4;
                            block = (ShadowBlock *)
                                ((u8 *) sector->blocksC + blockOffset);
                        } while ((vertex < block->lastVertex18) &&
                                 (done == 0));
                    }
                }
                blockOffset += 0x10;
                block = (ShadowBlock *) ((u8 *) block + 0x10);
                blockNumber++;
            } while ((blockNumber < sector->blockCount24) && (done == 0));
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/shadows/func_800180B4.s")
#endif
