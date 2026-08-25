#include "PR/ultratypes.h"

#define M2C_FIELD(expr, type_ptr, offset) \
    (*(type_ptr)((u8 *)(expr) + (offset)))

s32 o50UnresolvedCallReloc(); /* extern */
void overlay50SubmitTimeGlyphs(s32, s32, s32, s32); /* extern */
extern u8 D_10[];
extern u8 D_12C[];
extern u8 D_1CC[];
extern s16 D_21C;
extern u8 D_230[];
extern u8 D_260[];
extern u8 D_290[];
extern u8 D_2B0[];
extern u8 D_2C0[];
extern u8 D_2E0[];
extern u8 D_300[];
extern u8 D_328[];
extern s8 D_32C;
extern s32 D_334;
extern s32 D_33C;
extern s32 D_38;
extern u8 D_6C[];
extern u8 D_90[];
extern f32 D_A4;
extern f32 D_A8;
extern u8 D_AC[];
extern s32 D_B0[];
extern s32 D_BC;
extern s32 D_C0;
extern s16 D_C8;
extern s16 D_CA;
extern u8 D_DC[];
extern u8 D_FC[];
extern s32 o50UnresolvedGlobalReloc;
extern s16 o50UnresolvedS16TableReloc[];
extern s32 o50UnresolvedS32TableReloc[];

/* Exact donor scans are negative; this is a Mickey-only m2c control-flow
 * reconstruction pending relocation-name and type recovery. */
/* NON_MATCHING plateau: the full flag lattice favors -O2 -g3 -mips2 with
 * loop unrolling disabled. That form is 0x1890 bytes versus the target's
 * 0x189C and has 1,544 positional word differences; the first mismatch is
 * +0x0 because the reconstructed locals grow the frame from 0x118 to 0x1A0.
 * The closest reference skeleton is only 0.064 Jaccard, and the unresolved
 * overlay relocation identities prevent reliable signature/type recovery. */
#ifdef NON_MATCHING
void func_overlay_050_F0000334_1896CA4(void *arg0, s32 arg1) {
    s32 sp114;
    s32 *sp10C;
    s32 *sp108;
    s32 sp100;
    s32 spFC;
    s32 spAC;
    s32 sp9C;
    s16 sp9A;
    s16 sp98;
    s32 sp94;
    s32 sp90;
    s32 sp8C;
    u8 *sp88;
    void *sp84;
    u8 *sp80;
    s32 sp7C;
    s32 sp74;
    s32 sp70;
    s32 sp6C;
    s32 sp68;
    s32 sp64;
    s32 sp3C;
    s32 sp38;                                       /* compiler-managed */
    s32 sp34;
    void *sp30;
    u8 *var_v0_5;
    u8 *var_v0_6;
    u8 *var_v0_7;
    u8 *var_v1;
    f32 temp_f0;
    f32 var_f10;
    f32 var_f10_2;
    f32 var_f6;
    f32 var_f6_2;
    s16 temp_a0_2;
    s16 temp_v0_10;
    s16 temp_v0_11;
    s16 temp_v0_12;
    s16 temp_v0_13;
    s16 temp_v0_14;
    s16 var_v0_2;
    s16 var_v0_4;
    s32 *var_v1_3;
    s32 temp_a0;
    s32 temp_a0_4;
    s32 temp_a0_5;
    s32 temp_a0_6;
    s32 temp_t6;
    s32 temp_t6_2;
    s32 temp_t6_3;
    s32 temp_t6_4;
    s32 temp_t7_2;
    s32 temp_t7_3;
    s32 temp_t8;
    s32 temp_t8_2;
    s32 temp_t8_3;
    s32 temp_t8_4;
    s32 temp_t9;
    s32 temp_t9_2;
    s32 temp_t9_3;
    s32 temp_t9_4;
    s32 temp_t9_5;
    s32 temp_t9_6;
    s32 temp_v0;
    s32 temp_v0_15;
    s32 temp_v0_16;
    s32 temp_v0_17;
    s32 temp_v0_5;
    s32 temp_v0_6;
    s32 temp_v0_7;
    s32 temp_v0_8;
    s32 temp_v0_9;
    s32 temp_v1_2;
    s32 temp_v1_3;
    s32 var_a0;
    s32 var_a2;
    s32 var_a2_2;
    s32 var_t1;
    s32 var_t1_2;
    s32 var_t1_3;
    s32 var_t1_4;
    s32 var_t1_5;
    s32 var_t1_6;
    s32 var_t1_7;
    s32 var_t1_8;
    s32 var_v0;
    s32 var_v0_3;
    s32 var_v1_2;
    s8 temp_a0_3;
    s8 temp_t7;
    s8 temp_v0_3;
    s8 var_t2;
    u8 *temp_v0_2;
    u8 temp_v0_18;
    u8 temp_v0_4;
    u8 temp_v1;
    u8 *var_t0;

    sp80 = (u8 *)o50UnresolvedCallReloc();
    if (arg0 != NULL) {
        sp84 = M2C_FIELD(arg0, void **, 0x64);
        o50UnresolvedCallReloc(0, 0);
        temp_a0 = *(s32 *)0xC4;
        if (temp_a0 != 0) {
            D_C8 += arg1;
            if (D_C8 >= 0x3D) {
                if (D_C8 >= 0xF1) {
                    D_CA -= arg1 * 4;
                    if (D_CA < 0) {
                        o50UnresolvedCallReloc(temp_a0, D_CA);
                        *(s32 *)0xC4 = 0;
                    } else {
                        o50UnresolvedCallReloc(temp_a0, D_CA);
                    }
                } else {
                    D_CA += arg1 * 4;
                    if (D_CA >= 0x100) {
                        D_CA = 0xFF;
                    }
                    o50UnresolvedCallReloc(temp_a0, D_CA);
                }
            }
        }
        if (o50UnresolvedGlobalReloc == 0) {
            var_t1 = 0;
            if (arg1 > 0) {
                temp_v0 = arg1 & 3;
                if (temp_v0 != 0) {
                    var_t1 = 1;
                    var_f6 = 0.0f - D_A8;
                    var_f10 = (0.0f - D_A4) * 0.125f;
                    if (temp_v0 != 1) {
                        do {
                            var_t1 += 1;
                            D_A4 += var_f10;
                            D_A8 += var_f6 * 0.125f;
                            var_f10 = (0.0f - D_A4) * 0.125f;
                            var_f6 = 0.0f - D_A8;
                        } while (temp_v0 != var_t1);
                    }
                    D_A4 += var_f10;
                    D_A8 += var_f6 * 0.125f;
                    if (var_t1 != arg1) {
                        goto block_16;
                    }
                } else {
block_16:
                    var_t1_2 = var_t1 + 4;
                    var_f6_2 = 0.0f - D_A8;
                    var_f10_2 = (0.0f - D_A4) * 0.125f;
                    if (var_t1_2 != arg1) {
                        do {
                            var_t1_2 += 4;
                            D_A4 += var_f10_2;
                            D_A8 += var_f6_2 * 0.125f;
                            D_A4 += (0.0f - D_A4) * 0.125f;
                            D_A8 += (0.0f - D_A8) * 0.125f;
                            D_A4 += (0.0f - D_A4) * 0.125f;
                            D_A8 += (0.0f - D_A8) * 0.125f;
                            D_A4 += (0.0f - D_A4) * 0.125f;
                            D_A8 += (0.0f - D_A8) * 0.125f;
                            var_f10_2 = (0.0f - D_A4) * 0.125f;
                            var_f6_2 = 0.0f - D_A8;
                        } while (var_t1_2 != arg1);
                    }
                    D_A4 += var_f10_2;
                    D_A8 += var_f6_2 * 0.125f;
                    D_A4 += (0.0f - D_A4) * 0.125f;
                    D_A8 += (0.0f - D_A8) * 0.125f;
                    D_A4 += (0.0f - D_A4) * 0.125f;
                    D_A8 += (0.0f - D_A8) * 0.125f;
                    D_A4 += (0.0f - D_A4) * 0.125f;
                    D_A8 += (0.0f - D_A8) * 0.125f;
                }
            }
        }
        sp68 = (s32) D_A4;
        sp64 = (s32) D_A8;
        M2C_FIELD(&D_FC, s32 *, 8) = (s32) (((s32) M2C_FIELD(sp84, u8 *, 0x192) / 10) << 0x10);
        M2C_FIELD(&D_FC, s32 *, 0x18) = (s32) (((s32) M2C_FIELD(sp84, u8 *, 0x192) % 10) << 0x10);
        temp_a0_2 = M2C_FIELD(sp84, s16 *, 0x3BA);
        if (temp_a0_2 != 0xFF) {
            *(s32 *)0xB4 = temp_a0_2 << 0x10;
            *(s32 *)0xC4 = (s32) (M2C_FIELD(M2C_FIELD(sp84, s16 *, 0x3BA), s8 *, 0x320) << 0x10);
        } else {
            *(s32 *)0xB4 = (s32) (M2C_FIELD(sp84, u8 *, 0x385) << 0x10);
            *(s32 *)0xC4 = (s32) (M2C_FIELD(M2C_FIELD(sp84, u8 *, 0x385), s8 *, 0x320) << 0x10);
        }
        o50UnresolvedCallReloc(M2C_FIELD(sp84, s32 *, 0x400), (s16) &sp74, &sp70, &sp6C);
        temp_v0_2 = (u8 *)o50UnresolvedCallReloc();
        sp88 = temp_v0_2;
        if ((o50UnresolvedGlobalReloc == 0) && (M2C_FIELD(temp_v0_2, s8 *, 0x86) != M2C_FIELD(sp84, s8 *, 0x383)) && (o50UnresolvedCallReloc() == 0) && (o50UnresolvedCallReloc((s32) arg0) != M2C_FIELD(sp84, s32 *, 0x400))) {
            temp_v0_3 = D_32C;
            D_32C = temp_v0_3 + 1;
            sp6C = (sp6C - (sp6C % 10)) + temp_v0_3;
            D_32C = (s8) ((s8) D_32C % 10);
        }
        M2C_FIELD(&D_12C, s32 *, 8) = (s32) ((sp74 / 10) << 0x10);
        M2C_FIELD(&D_12C, s32 *, 0x18) = (s32) ((sp74 % 10) << 0x10);
        M2C_FIELD(&D_12C, s32 *, 0x38) = (s32) ((sp70 / 10) << 0x10);
        M2C_FIELD(&D_12C, s32 *, 0x48) = (s32) ((sp70 % 10) << 0x10);
        M2C_FIELD(&D_12C, s32 *, 0x68) = (s32) ((sp6C / 10) << 0x10);
        M2C_FIELD(&D_12C, s32 *, 0x78) = (s32) ((sp6C % 10) << 0x10);
        o50UnresolvedCallReloc(0, (s16) &D_12C, NULL, (s32 *) sp68, 0xFF, (s32 **)0xFF, (s32 **)0xFF, 0xFF);
        o50UnresolvedCallReloc(0, (s16) &D_FC, NULL, (s32 *) sp68, 0xFF, (s32 **)0xFF, (s32 **)0xFF, 0xFF);
        if (*sp80 != 1) {
            if (M2C_FIELD(sp84, u16 *, 0x1A8) & 8) {
                o50UnresolvedCallReloc(0, (s16) &D_DC, NULL, (s32 *) sp68, 0xFF, (s32 **)0xFF, (s32 **)0xFF, 0xFF);
            } else {
                o50UnresolvedCallReloc(0, (s16) &D_AC, NULL, (s32 *) sp68, 0xFF, (s32 **)0xFF, (s32 **)0xFF, 0xFF);
            }
        }
        o50UnresolvedCallReloc(0);
        *(f32 *)0x90 = (f32) (0x54 - sp68);
        *(f32 *)0x84 = (s16) ((s32) (M2C_FIELD(sp84, s32 *, 0x400) * -0x10000) / 300);
        o50UnresolvedCallReloc(4);
        *(f32 *)0x30 = (f32) (0x43 - sp68);
        o50UnresolvedCallReloc(1);
        o50UnresolvedCallReloc(0, (s16) &D_2C0, NULL, (s32 *) sp68, 0xFF, (s32 **)0xFF, (s32 **)0xFF, 0xFF);
        o50UnresolvedCallReloc(0);
        if (M2C_FIELD(sp84, u8 *, 0x19A) != 0xFF) {
            var_v0 = D_334 + (arg1 * 0x10);
            D_334 = var_v0;
            if (var_v0 >= 0x100) {
                var_v0 = 0xFF;
                D_334 = 0xFF;
            }
        } else {
            var_v0 = D_334 - (arg1 * 8);
            D_334 = var_v0;
            if (var_v0 < 0) {
                D_334 = 0;
                var_v0 = 0;
            }
        }
        if (var_v0 > 0) {
            if (M2C_FIELD(sp84, s32 *, 0x19C) != 0) {
                var_v0_2 = 0x35;
                sp38 = (s32) *(s8 *)0x330;
            } else {
                temp_v1 = M2C_FIELD(sp84, u8 *, 0x19A);
                if (temp_v1 != 0xFF) {
                    var_v0_2 = o50UnresolvedS16TableReloc[temp_v1];
                    sp38 = (s32) *(s8 *)0x330;
                } else {
                    var_v0_2 = (s16) *(s8 *)0x330;
                    sp38 = (s32) var_v0_2;
                }
            }
            if (var_v0_2 != sp38) {
                if (sp38 != -1) {
                    sp100 = (s32) var_v0_2;
                    o50UnresolvedCallReloc(sp38);
                }
                *(s8 *)0x330 = (s8) var_v0_2;
                temp_t7 = *(s8 *)0x330;
                sp38 = (s32) temp_t7;
                if (temp_t7 != -1) {
                    o50UnresolvedCallReloc((s32) temp_t7);
                    sp38 = (s32) *(s8 *)0x330;
                }
            }
            if (sp38 != -1) {
                if (sp38 == 0x35) {
                    sp98 = 0x8A;
                    sp9A = 0xF;
                } else {
                    sp98 = 0x90;
                    sp9A = 0x15;
                }
                sp94 = 0;
                sp90 = 0;
                sp9C = 0;
                sp8C = o50UnresolvedS32TableReloc[sp38];
                o50UnresolvedCallReloc(0, (s16) &sp8C, NULL, NULL, 0xFF, (s32 **)0xFF, (s32 **)0xFF, D_334);
                if (*(s8 *)0x330 != 0x35) {
                    temp_v0_4 = M2C_FIELD(sp84, u8 *, 0x19B);
                    if ((s32) temp_v0_4 >= 2) {
                        M2C_FIELD(&D_300, s32 *, 8) = (s32) (temp_v0_4 << 0x10);
                        o50UnresolvedCallReloc(0, (s16) &D_300, (s32 *)0xAB, (s32 *)0x2E, 0, NULL, NULL, D_334);
                        o50UnresolvedCallReloc(0, (s16) &D_300, (s32 *)0xAC, (s32 *)0x30, 0, NULL, NULL, D_334);
                        o50UnresolvedCallReloc(0, (s16) &D_300, (s32 *)0xAC, (s32 *)0x2F, 0xFF, (s32 **)0xFF, (s32 **)0xFF, D_334);
                    }
                }
            }
        } else {
            temp_a0_3 = *(s8 *)0x330;
            if (temp_a0_3 != -1) {
                o50UnresolvedCallReloc((s32) temp_a0_3);
                *(s8 *)0x330 = -1;
            }
        }
        if (M2C_FIELD(sp84, u8 *, 0x388) != 0) {
            var_t1_3 = 0;
            if (arg1 > 0) {
                temp_v0_5 = arg1 & 3;
                if (temp_v0_5 != 0) {
                    do {
                        var_t1_3 += 1;
                        D_21C += (s32) (0x800 - D_21C) >> 3;
                    } while (temp_v0_5 != var_t1_3);
                    if (var_t1_3 != arg1) {
                        goto block_62;
                    }
                } else {
block_62:
                    do {
                        var_t1_3 += 4;
                        D_21C += (s32) (0x800 - D_21C) >> 3;
                        D_21C += (s32) (0x800 - D_21C) >> 3;
                        D_21C += (s32) (0x800 - D_21C) >> 3;
                        D_21C += (s32) (0x800 - D_21C) >> 3;
                    } while (var_t1_3 != arg1);
                }
            }
            var_v0_3 = 1;
        } else if (D_21C == -0x420) {
            var_v0_3 = 0;
        } else {
            var_t1_4 = 0;
            if (arg1 > 0) {
                temp_v0_6 = arg1 & 3;
                if (temp_v0_6 != 0) {
                    do {
                        var_t1_4 += 1;
                        D_21C += (s32) (0x1820 - D_21C) >> 3;
                    } while (temp_v0_6 != var_t1_4);
                    if (var_t1_4 != arg1) {
                        goto block_72;
                    }
                } else {
block_72:
                    do {
                        var_t1_4 += 4;
                        D_21C += (s32) (0x1820 - D_21C) >> 3;
                        D_21C += (s32) (0x1820 - D_21C) >> 3;
                        D_21C += (s32) (0x1820 - D_21C) >> 3;
                        D_21C += (s32) (0x1820 - D_21C) >> 3;
                    } while (var_t1_4 != arg1);
                }
            }
            var_v0_3 = 1;
            if (((s16) D_21C >> 6) == 0x60) {
                D_21C = -0x420;
                var_v0_3 = 0;
            }
        }
        if (var_v0_3 != 0) {
            temp_t9 = (s16) D_21C >> 4;
            M2C_FIELD(&D_1CC, s16 *, 0xC) = (s16) temp_t9;
            M2C_FIELD(&D_1CC, s16 *, 0x1C) = (s16) temp_t9;
            M2C_FIELD(&D_1CC, s16 *, 0x2C) = (s16) temp_t9;
            M2C_FIELD(&D_1CC, s16 *, 0x3C) = (s16) temp_t9;
            o50UnresolvedCallReloc(0, (s16) &D_1CC, NULL, NULL, 0xFF, (s32 **)0xFF, (s32 **)0xFF, 0xC0);
        }
        if (o50UnresolvedCallReloc(0, -0x18, (s32 *)0xBE, (s32 *)0x30, 0xBE, &sp10C, &sp108, 1) != 0) {
            o50UnresolvedCallReloc(0, (s16) &spAC);
            if (spAC != 0) {
                o50UnresolvedCallReloc(0, (s16) &spAC, sp10C, sp108, 0xFF, (s32 **)0xFF, (s32 **)0xFF, 0xFF);
            }
            o50UnresolvedCallReloc(0, 0, sp10C, sp108);
        }
        o50UnresolvedCallReloc(o50UnresolvedGlobalReloc, (s16) &D_328, (s32 *)0x14, &D_38, arg1);
        if (M2C_FIELD(sp84, s8 *, 0x383) < M2C_FIELD(sp88, s8 *, 0x86)) {
            var_v0_4 = M2C_FIELD(sp84, s16 *, 0x456);
            if (var_v0_4 >= arg1) {
                var_t1_5 = 0;
                if ((var_v0_4 == 0xB4) && (M2C_FIELD(sp84, s16 *, 0x454) >= 0)) {
                    sp114 = 0;
                    o50UnresolvedCallReloc(0x1F8, 0);
                    var_t1_5 = 0;
                    var_v0_4 = M2C_FIELD(sp84, s16 *, 0x456);
                }
                M2C_FIELD(sp84, s16 *, 0x456) = (s16) (var_v0_4 - arg1);
                if (arg1 > 0) {
                    temp_v0_7 = arg1 & 3;
                    if (temp_v0_7 != 0) {
                        do {
                            var_t1_5 += 1;
                            D_C0 += (s32) (0x550 - D_C0) >> 3;
                            D_BC += (s32) (0x830 - D_BC) >> 3;
                        } while (temp_v0_7 != var_t1_5);
                        if (var_t1_5 != arg1) {
                            goto block_93;
                        }
                    } else {
block_93:
                        do {
                            var_t1_5 += 4;
                            temp_t9_2 = D_BC + ((s32) (0x830 - D_BC) >> 3);
                            D_BC = temp_t9_2;
                            temp_t6 = D_C0 + ((s32) (0x550 - D_C0) >> 3);
                            temp_t7_2 = temp_t9_2 + ((s32) (0x830 - temp_t9_2) >> 3);
                            D_C0 = temp_t6;
                            temp_t8 = temp_t6 + ((s32) (0x550 - temp_t6) >> 3);
                            D_BC = temp_t7_2;
                            temp_t9_3 = temp_t7_2 + ((s32) (0x830 - temp_t7_2) >> 3);
                            D_C0 = temp_t8;
                            temp_t6_2 = temp_t8 + ((s32) (0x550 - temp_t8) >> 3);
                            D_BC = temp_t9_3;
                            D_C0 = temp_t6_2;
                            D_C0 = temp_t6_2 + ((s32) (0x550 - temp_t6_2) >> 3);
                            D_BC = temp_t9_3 + ((s32) (0x830 - temp_t9_3) >> 3);
                        } while (var_t1_5 != arg1);
                    }
                }
            } else if (D_BC != -0x500) {
                var_t1_6 = 0;
                if (var_v0_4 != -1) {
                    sp114 = 0;
                    o50UnresolvedCallReloc(0x1F9, 0);
                    var_t1_6 = 0;
                    M2C_FIELD(sp84, s16 *, 0x456) = -1;
                }
                temp_v0_8 = arg1 & 3;
                if (arg1 > 0) {
                    if (temp_v0_8 != 0) {
                        do {
                            var_t1_6 += 1;
                            D_C0 += (s32) (-0x140 - D_C0) >> 3;
                            D_BC += (s32) (0x1900 - D_BC) >> 3;
                        } while (temp_v0_8 != var_t1_6);
                        if (var_t1_6 != arg1) {
                            goto block_104;
                        }
                    } else {
block_104:
                        do {
                            var_t1_6 += 4;
                            temp_t8_2 = D_BC + ((s32) (0x1900 - D_BC) >> 3);
                            D_BC = temp_t8_2;
                            temp_t9_4 = D_C0 + ((s32) (-0x140 - D_C0) >> 3);
                            temp_t6_3 = temp_t8_2 + ((s32) (0x1900 - temp_t8_2) >> 3);
                            D_C0 = temp_t9_4;
                            temp_t7_3 = temp_t9_4 + ((s32) (-0x140 - temp_t9_4) >> 3);
                            D_BC = temp_t6_3;
                            temp_t8_3 = temp_t6_3 + ((s32) (0x1900 - temp_t6_3) >> 3);
                            D_C0 = temp_t7_3;
                            temp_t9_5 = temp_t7_3 + ((s32) (-0x140 - temp_t7_3) >> 3);
                            D_BC = temp_t8_3;
                            D_C0 = temp_t9_5;
                            D_C0 = temp_t9_5 + ((s32) (-0x140 - temp_t9_5) >> 3);
                            D_BC = temp_t8_3 + ((s32) (0x1900 - temp_t8_3) >> 3);
                        } while (var_t1_6 != arg1);
                    }
                }
                if (D_BC >= 0x1861) {
                    D_BC = -0x500;
                    D_C0 = -0x140;
                }
            }
            if (M2C_FIELD(sp84, s16 *, 0x454) <= 0) {
                *(f32 *)8 = 1.102026e-39f;
                temp_v1_2 = *(s32 *)0x54;
                temp_a0_4 = *(s32 *)0x50;
                sp7C = (s32) -M2C_FIELD(sp84, s16 *, 0x454);
                var_v0_5 = D_10;
                *(s32 *)4 = temp_v1_2;
                o50UnresolvedGlobalReloc = temp_a0_4;
                do {
                    var_v0_5 += 0x40;
                    M2C_FIELD(var_v0_5, s32 *, -0x30) = temp_a0_4;
                    M2C_FIELD(var_v0_5, s32 *, -0x2C) = temp_v1_2;
                    M2C_FIELD(var_v0_5, s32 *, -0x20) = temp_a0_4;
                    M2C_FIELD(var_v0_5, s32 *, -0x1C) = temp_v1_2;
                    M2C_FIELD(var_v0_5, s32 *, -0x10) = temp_a0_4;
                    M2C_FIELD(var_v0_5, s32 *, -0xC) = temp_v1_2;
                    M2C_FIELD(var_v0_5, s32 *, -0x40) = temp_a0_4;
                    M2C_FIELD(var_v0_5, s32 *, -0x3C) = temp_v1_2;
                } while (var_v0_5 != D_90);
            } else {
                *(f32 *)8 = 1.193861e-39f;
                temp_v1_3 = *(s32 *)0x54;
                temp_a0_5 = *(s32 *)0x140;
                var_v0_6 = D_10;
                *(s32 *)4 = temp_v1_3;
                o50UnresolvedGlobalReloc = temp_a0_5;
                sp7C = (s32) M2C_FIELD(sp84, s16 *, 0x454);
                do {
                    var_v0_6 += 0x40;
                    M2C_FIELD(var_v0_6, s32 *, -0x30) = temp_a0_5;
                    M2C_FIELD(var_v0_6, s32 *, -0x2C) = temp_v1_3;
                    M2C_FIELD(var_v0_6, s32 *, -0x20) = temp_a0_5;
                    M2C_FIELD(var_v0_6, s32 *, -0x1C) = temp_v1_3;
                    M2C_FIELD(var_v0_6, s32 *, -0x10) = temp_a0_5;
                    M2C_FIELD(var_v0_6, s32 *, -0xC) = temp_v1_3;
                    M2C_FIELD(var_v0_6, s32 *, -0x40) = temp_a0_5;
                    M2C_FIELD(var_v0_6, s32 *, -0x3C) = temp_v1_3;
                } while (var_v0_6 != D_90);
            }
            o50UnresolvedCallReloc(sp7C, (s16) &sp74, &sp70, &sp6C);
            *(s32 *)0x18 = (sp74 / 10) << 0x10;
            *(s32 *)0x28 = (sp74 % 10) << 0x10;
            *(s32 *)0x48 = (sp70 / 10) << 0x10;
            *(s32 *)0x58 = (sp70 % 10) << 0x10;
            *(s32 *)0x78 = (sp6C / 10) << 0x10;
            *(s32 *)0x88 = (sp6C % 10) << 0x10;
            var_v1 = D_230;
            var_v0_7 = D_10;
            do {
                if (((s32) M2C_FIELD(var_v0_7, s32 *, 8) >> 0x10) == 1) {
                    if ((var_v1 == D_230) || (var_v1 == D_260) || (var_v1 == D_290)) {
                        M2C_FIELD(var_v0_7, s16 *, 0xC) = (s16) (M2C_FIELD(var_v1, s16 *, 0xC) + 1);
                    } else {
                        M2C_FIELD(var_v0_7, s16 *, 0xC) = (s16) (M2C_FIELD(var_v1, s16 *, 0xC) - 1);
                    }
                } else {
                    M2C_FIELD(var_v0_7, s16 *, 0xC) = (s16) M2C_FIELD(var_v1, s16 *, 0xC);
                }
                var_v1 += 0x10;
                var_v0_7 += 0x10;
            } while (var_v1 != D_2B0);
            o50UnresolvedCallReloc(0, 0, (s32 *) ((s32) D_BC >> 4), (s32 *) ((s32) D_C0 >> 4), 0xFF, (s32 **)0xFF, (s32 **)0xFF, 0xFF);
        }
        sp114 = 0;
        var_t1_7 = sp114;
        if (o50UnresolvedCallReloc(0) & 1) {
            o50UnresolvedGlobalReloc ^= 1;
        }
        if (o50UnresolvedGlobalReloc != 0) {
            if ((u16)o50UnresolvedGlobalReloc & 1) {
                var_a2 = (s32) (M2C_FIELD(sp84, f32 *, 4) * 6.25f);
                if (var_a2 < 0) {
                    var_a2 = -var_a2;
                }
                o50UnresolvedCallReloc(0xE6, 0xB4, (s32 *) var_a2, (s32 *)3, 0);
                o50UnresolvedCallReloc(2);
                o50UnresolvedCallReloc(0, 0, NULL, NULL);
                o50UnresolvedCallReloc(0x40, 0xFF, (s32 *)0x40, (s32 *)0xFF, 0xE0);
                o50UnresolvedCallReloc(0, 0x106, (s32 *)0xBC, NULL, 0);
            } else {
                o50UnresolvedCallReloc(0, (s16) &D_6C, (s32 *) sp64, NULL, 0xFF, (s32 **)0xFF, (s32 **)0xFF, 0xFF);
                temp_f0 = M2C_FIELD(sp84, f32 *, 4);
                var_t1_8 = var_t1_7;
                temp_v0_9 = arg1 & 3;
                if (temp_f0 < 0.0f) {
                    var_v1_2 = (s32) (16384.0f - (-temp_f0 * *(f32 *)8));
                } else {
                    var_v1_2 = (s32) (16384.0f - (temp_f0 * *(f32 *)0xC));
                }
                if (arg1 > 0) {
                    if (temp_v0_9 != 0) {
                        do {
                            temp_v0_10 = (s16) *(f32 *)4;
                            var_t1_8 += 1;
                            *(f32 *)4 = (s16) (temp_v0_10 + ((s32) (var_v1_2 - temp_v0_10) >> 2));
                        } while (temp_v0_9 != var_t1_8);
                        if (var_t1_8 != arg1) {
                            goto loop_138;
                        }
                    } else {
                        do {
loop_138:
                            temp_v0_11 = (s16) *(f32 *)4;
                            var_t1_8 += 4;
                            *(f32 *)4 = (s16) (temp_v0_11 + ((s32) (var_v1_2 - temp_v0_11) >> 2));
                            temp_v0_12 = (s16) *(f32 *)4;
                            *(f32 *)4 = (s16) (temp_v0_12 + ((s32) (var_v1_2 - temp_v0_12) >> 2));
                            temp_v0_13 = (s16) *(f32 *)4;
                            *(f32 *)4 = (s16) (temp_v0_13 + ((s32) (var_v1_2 - temp_v0_13) >> 2));
                            temp_v0_14 = (s16) *(f32 *)4;
                            *(f32 *)4 = (s16) (temp_v0_14 + ((s32) (var_v1_2 - temp_v0_14) >> 2));
                        } while (var_t1_8 != arg1);
                    }
                    var_t1_8 = 0;
                }
                sp114 = var_t1_8;
                *(f32 *)0x10 = -85.0f;
                *(f32 *)0xC = (f32) (sp64 + 0x77);
                o50UnresolvedCallReloc(0);
                o50UnresolvedGlobalReloc = 0xFF;
                o50UnresolvedCallReloc(0);
                var_t1_7 = var_t1_8;
                o50UnresolvedGlobalReloc = 0xFF;
            }
        }
        var_v1_3 = D_B0;
        var_a2_2 = 0x3C;
        var_t0 = sp84;
        var_t2 = M2C_FIELD(sp84, s8 *, 0x383);
        if (M2C_FIELD(sp84, u8 *, 0x45C) != 0) {
            var_t2 += 1;
        }
        if (var_t2 > 0) {
            do {
                var_a0 = 0;
                if (arg1 > 0) {
                    temp_v0_15 = arg1 & 3;
                    if (temp_v0_15 != 0) {
                        do {
                            temp_v0_16 = *var_v1_3;
                            var_a0 += 1;
                            *var_v1_3 = temp_v0_16 + ((s32) -temp_v0_16 >> 2);
                        } while (temp_v0_15 != var_a0);
                        if (var_a0 != arg1) {
                            goto loop_148;
                        }
                    } else {
                        do {
loop_148:
                            temp_v0_17 = *var_v1_3;
                            var_a0 += 4;
                            temp_t9_6 = temp_v0_17 + ((s32) -temp_v0_17 >> 2);
                            temp_t8_4 = temp_t9_6 + ((s32) -temp_t9_6 >> 2);
                            *var_v1_3 = temp_t9_6;
                            temp_t6_4 = temp_t8_4 + ((s32) -temp_t8_4 >> 2);
                            *var_v1_3 = temp_t8_4;
                            *var_v1_3 = temp_t6_4;
                            *var_v1_3 = temp_t6_4 + ((s32) -temp_t6_4 >> 2);
                        } while (var_a0 != arg1);
                    }
                }
                temp_a0_6 = var_t1_7 + 1;
                sp3C = temp_a0_6;
                spFC = (s32) var_t2;
                sp30 = var_t0;
                sp34 = var_a2_2;
                sp38 = var_v1_3;
                overlay50SubmitTimeGlyphs(temp_a0_6, *var_v1_3 + 0xD6, var_a2_2, M2C_FIELD(var_t0, s32 *, 0x404));
                var_t1_7 = sp3C;
                var_v1_3 += 4;
                var_a2_2 += 0xA;
                var_t0 += 4;
            } while (var_t1_7 != var_t2);
        }
        if (var_t2 > 0) {
            o50UnresolvedCallReloc(0, (s16) &D_2E0, *(s32 **)0xB0, NULL, 0xFF, (s32 **)0xFF, (s32 **)0xFF, 0xFF);
        }
        if (o50UnresolvedCallReloc() == 0) {
            temp_v0_18 = *sp80;
            switch (temp_v0_18) {                   /* irregular */
            case 0:
                if ((o50UnresolvedGlobalReloc == 0) && (o50UnresolvedCallReloc(0) & 0x9000) && (D_33C == 0)) {
                    o50UnresolvedCallReloc(1);
                    o50UnresolvedCallReloc();
                    o50UnresolvedCallReloc(2, 0x40800000, (s32 *)0xBF800000, NULL, 0, NULL, NULL);
                    o50UnresolvedCallReloc(0x12, 0, NULL, (s32 *)7, 1, (s32 **)1);
                    o50UnresolvedCallReloc(0x40400000, 0);
                    D_33C = 1;
                    return;
                }
                break;
            case 1:
                if ((o50UnresolvedGlobalReloc == 0) && (o50UnresolvedCallReloc(0) & 0x9000) && (D_33C == 0)) {
                    o50UnresolvedCallReloc(1);
                    o50UnresolvedCallReloc();
                    o50UnresolvedCallReloc(2, 0x40800000, (s32 *)0xBF800000, NULL, 0, NULL, NULL);
                    o50UnresolvedCallReloc(0x12, 0, NULL, (s32 *)7, 1, NULL);
                    o50UnresolvedCallReloc(0x40400000, 0);
                    D_33C = 1;
                }
                break;
            }
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o050/func_overlay_050_F0000334_1896CA4/func_overlay_050_F0000334_1896CA4.s")
#endif
