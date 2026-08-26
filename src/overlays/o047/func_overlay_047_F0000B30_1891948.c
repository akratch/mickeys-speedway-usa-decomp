#include "PR/ultratypes.h"

typedef s32 M2C_UNK;
#define M2C_FIELD(expr, type_ptr, offset) \
    (*(type_ptr)((s8 *)(expr) + (offset)))
#define M2C_BITWISE(type, expr) ((type)(expr))

/*
 * Mickey-only mips_to_c scaffold. The permitted reference scan found no
 * close donor: the nearest masked skeleton is JFG frontKeyboard at 0.057.
 * The extracted assembly does not retain external relocation identities, so
 * overlay47ExternalReloc represents multiple calls and makes no naming claim.
 * Workbench plateau (2026-08-26): structure-mismatch; typed-null and entry-lifetime variants still have 2,218 differing comparison words, 2,238 versus 2,168 instructions, and a -776 versus -640 frame.
 * Levers tried: constant-audit, removal of the artificial static NULL base, and entry float-temporary collapse; the null-base change reduced the residual but did not match.
 * Remains: frame surplus and unresolved call/data relocation plus aggregate-type structure.
 */
#ifdef NON_MATCHING
s16 overlay47ExternalReloc(); /* extern; the scaffold collapses several callees */
M2C_UNK func_overlay_047_F0002D10_1893B28(void *);  /* extern */
#undef NULL
#define NULL ((M2C_UNK *)0)
extern M2C_UNK *D_1C0;
extern M2C_UNK D_210;
extern M2C_UNK D_280;
extern M2C_UNK D_2A8;
extern M2C_UNK D_2F0;
extern M2C_UNK D_300;
extern s8 D_30A;
extern M2C_UNK *D_310;
extern M2C_UNK *D_31C;
extern M2C_UNK *D_320;
extern M2C_UNK D_328;
extern M2C_UNK *D_338;
extern M2C_UNK D_3CC;
extern M2C_UNK D_3DC;
extern M2C_UNK D_474;
extern M2C_UNK D_480;
extern M2C_UNK D_4F0;
extern f32 D_540;
extern f32 D_544;
extern M2C_UNK D_548;
extern f32 D_54C;
extern s32 D_550;
extern s32 D_554;
extern M2C_UNK D_8;
extern M2C_UNK D_80000000;
extern M2C_UNK D_8000008C;
extern M2C_UNK D_80000118;
extern M2C_UNK *D_80000198;
extern M2C_UNK *D_800001C0;
extern M2C_UNK *D_80000228;
extern M2C_UNK *D_80000268;
extern M2C_UNK D_D0;

void func_overlay_047_F0000B30_1891948(s32 arg0) {
    M2C_UNK *sp278;
    s32 sp258;
    s32 sp250;
    M2C_UNK *sp230;
    M2C_UNK *sp22C;
    s32 sp21C;
    s32 sp214;
    f32 sp1FC;
    f32 sp1F8;
    M2C_UNK *sp1B8;
    M2C_UNK *sp178;
    M2C_UNK *sp138;
    M2C_UNK *sp12C;
    M2C_UNK *sp8C;
    M2C_UNK *sp88;
    s32 sp84;
    M2C_UNK *sp80;
    M2C_UNK **temp_s7;
    M2C_UNK **var_t0;
    M2C_UNK *temp_a0;
    M2C_UNK *temp_a0_2;
    M2C_UNK *temp_a0_3;
    M2C_UNK *temp_a0_4;
    M2C_UNK *temp_a2_2;
    M2C_UNK *temp_a3_2;
    M2C_UNK *temp_s0;
    M2C_UNK *temp_s2;
    M2C_UNK *temp_s2_2;
    M2C_UNK *temp_s2_3;
    M2C_UNK *temp_s2_4;
    M2C_UNK *temp_t0;
    M2C_UNK *temp_t6;
    M2C_UNK *temp_t8_4;
    M2C_UNK *temp_v0_10;
    M2C_UNK *temp_v0_11;
    M2C_UNK *temp_v0_12;
    M2C_UNK *temp_v0_13;
    M2C_UNK *temp_v0_20;
    M2C_UNK *temp_v0_21;
    M2C_UNK *temp_v0_22;
    M2C_UNK *temp_v0_23;
    M2C_UNK *temp_v0_24;
    M2C_UNK *temp_v0_25;
    M2C_UNK *temp_v0_26;
    M2C_UNK *temp_v0_27;
    M2C_UNK *temp_v0_28;
    M2C_UNK *temp_v0_29;
    M2C_UNK *temp_v0_30;
    M2C_UNK *temp_v0_31;
    M2C_UNK *temp_v0_32;
    M2C_UNK *temp_v0_33;
    M2C_UNK *temp_v0_34;
    M2C_UNK *temp_v0_37;
    M2C_UNK *temp_v0_38;
    M2C_UNK *temp_v0_39;
    M2C_UNK *temp_v0_40;
    M2C_UNK *temp_v0_9;
    M2C_UNK *temp_v1_10;
    M2C_UNK *temp_v1_11;
    M2C_UNK *var_s2;
    M2C_UNK *var_s3;
    f32 temp_f0;
    f32 temp_f0_2;
    f32 temp_f0_3;
    f32 temp_f10;
    f32 temp_f10_2;
    f32 temp_f12;
    f32 temp_f12_2;
    f32 temp_f12_3;
    f32 temp_f12_4;
    f32 temp_f16;
    f32 temp_f16_2;
    f32 temp_f20;
    f32 temp_f20_2;
    f32 temp_f22;
    f32 temp_f24;
    f32 temp_f26;
    f32 temp_f26_2;
    f32 temp_f2;
    f32 temp_f2_2;
    f32 temp_f4;
    f32 temp_f4_2;
    f32 temp_f8;
    f32 temp_f8_2;
    f32 var_f0;
    f32 var_f14;
    s16 temp_v0_2;
    s16 temp_v0_3;
    s16 temp_v0_4;
    s16 temp_v0_5;
    s16 temp_v0_6;
    s16 temp_v0_8;
    s16 temp_v1_5;
    s16 temp_v1_6;
    s16 temp_v1_7;
    s16 temp_v1_8;
    s16 temp_v1_9;
    s32 *temp_v1_2;
    s32 temp_a0_6;
    s32 temp_a2;
    s32 temp_a3;
    s32 temp_s1;
    s32 temp_t6_3;
    s32 temp_t6_4;
    s32 temp_t7_2;
    s32 temp_t7_3;
    s32 temp_t7_4;
    s32 temp_t7_5;
    s32 temp_t7_6;
    s32 temp_t8;
    s32 temp_t8_2;
    s32 temp_t8_3;
    s32 temp_t9_2;
    s32 temp_t9_3;
    s32 temp_t9_4;
    s32 temp_t9_5;
    s32 temp_t9_7;
    s32 temp_v0_14;
    s32 temp_v0_15;
    s32 temp_v0_16;
    s32 temp_v0_17;
    s32 temp_v0_18;
    s32 temp_v0_19;
    s32 temp_v0_36;
    s32 temp_v1_3;
    s32 temp_v1_4;
    s32 var_a0;
    s32 var_a0_2;
    s32 var_a0_3;
    s32 var_a1;
    s32 var_fp_2;
    s32 var_s0_2;
    s32 var_s1;
    s32 var_s1_2;
    s32 var_s1_3;
    s32 var_s1_4;
    s32 var_s4;
    s32 var_s4_2;
    s32 var_s4_3;
    s32 var_s4_4;
    s32 var_s4_5;
    s32 var_s4_6;
    s32 var_s5;
    s32 var_s6;
    s32 var_s6_2;
    s32 var_s7;
    s32 var_v0;
    s8 *temp_a1;
    s8 *temp_v0_35;
    s8 *var_v1;
    s8 *var_v1_2;
    s8 *var_v1_3;
    s8 temp_a0_5;
    s8 temp_t6_2;
    s8 temp_t7;
    s8 temp_t9_6;
    s8 temp_v0;
    s8 temp_v0_7;
    u8 temp_t9;
    void *temp_v1;
    s8 *var_a1_2;
    s8 *var_fp;
    s8 *var_fp_3;
    s8 *var_s0;

    sp1FC = (f32) arg0;
    *(s8 *)0x30B = 0;
    sp21C = 0;
    var_a1 = 1;
    sp250 = 0;
    var_v0 = 0;
    sp214 = 0;
    var_fp = NULL;
    do {
        if (M2C_FIELD(var_fp, s8 *, 0x2B) != 0) {
            var_v0 += 1;
            if (M2C_FIELD(var_fp, s8 *, 0x2A) == 0) {
                var_a1 = 0;
                D_338 = NULL;
            }
        }
        var_fp += 0x34;
    } while ((u32) var_fp < (u32) &D_D0);
    var_s6 = 0;
    if ((var_a1 != 0) && ((var_v0 == 1) || (var_v0 == 4))) {
        D_338 = (M2C_UNK *)1;
    }
    temp_f26 = *(f32 *)4;
    var_s0 = NULL;
    var_s3 = &D_8;
    temp_f22 = *(f32 *)8;
    sp278 = NULL;
    sp258 = var_a1;
    do {
        if (M2C_FIELD(var_s0, s8 *, 0x2B) == 0) {
            if ((overlay47ExternalReloc(sp278) & 0x9000) && (M2C_FIELD(var_s0, s8 *, 0x2C) == 0) && (*(s32 *)0x324 == 0) && (sp250 == 0)) {
                M2C_FIELD(var_s0, s8 *, 0x2B) = 1;
                M2C_FIELD(var_s0, f32 *, 0) = (f32) M2C_FIELD(&D_2F0, f32 *, 0);
                var_v1 = &D_300 + M2C_FIELD(var_s0, s16 *, 0x28);
                var_s4 = 0;
                M2C_FIELD(var_s0, f32 *, 4) = (f32) M2C_FIELD(&D_2F0, f32 *, 4);
                M2C_FIELD(var_s0, f32 *, 8) = (f32) M2C_FIELD(&D_2F0, f32 *, 8);
                M2C_FIELD(var_s0, s16 *, 0x20) = (s16) M2C_FIELD(&D_2F0, s16 *, 0xC);
                sp258 = 0;
                if (*var_v1 != 0) {
                    do {
                        M2C_FIELD(var_s0, s16 *, 0x28) = (s16) (M2C_FIELD(var_s0, s16 *, 0x28) + 1);
                        if (M2C_FIELD(var_s0, s16 *, 0x28) >= 0xA) {
                            M2C_FIELD(var_s0, s16 *, 0x28) = 0;
                        }
                        var_v1 = &D_300 + M2C_FIELD(var_s0, s16 *, 0x28);
                    } while (*var_v1 != 0);
                }
                *var_v1 = 1;
                *NULL += 1;
                D_30A += 1;
                overlay47ExternalReloc((M2C_UNK *)0xC, NULL, NULL);
                overlay47ExternalReloc((M2C_UNK *)0x19, NULL);
                temp_s2 = M2C_FIELD(var_s0, M2C_UNK **, 0x24);
                if (temp_s2 != NULL) {
                    if (M2C_FIELD(temp_s2, s16 *, 0x46) != M2C_FIELD((M2C_FIELD(var_s0, s16 *, 0x28) * 2), s16 *, 0x510)) {
                        overlay47ExternalReloc(temp_s2, (M2C_UNK *)1);
                        func_overlay_047_F0002D10_1893B28(var_s0);
                    } else {
                        overlay47ExternalReloc(temp_s2, (M2C_UNK *)1, (M2C_UNK *)-1, NULL);
                    }
                }
                temp_v0 = (s8) *NULL;
                if (temp_v0 > 0) {
                    do {
                        var_s4 += 1;
                        if (M2C_FIELD(var_s3, f32 *, 0x28) == (f32) M2C_FIELD(var_s0, s16 *, 0x28)) {
                            *(&D_328 + ((s32) sp278 * 4)) = ((s32) M2C_FIELD(var_s3, f32 *, 0xC) + 0xA0) * 0x10;
                        }
                        var_s3 += 0x2C;
                    } while (var_s4 < temp_v0);
                    var_s3 = &D_8;
                }
            }
        } else {
            if ((sp258 != 0) && (D_338 == NULL)) {
                sp21C = 1;
                if (*sp278 < -0x10) {
                    if ((u8) *NULL != 0) {
                        overlay47ExternalReloc((M2C_UNK *)0xE, NULL, &D_338);
                    } else {
                        *(s32 *)0x30B = 1;
                        *NULL = 1;
                    }
                }
                if (*sp278 >= 0x11) {
                    if ((u8) *NULL == 0) {
                        overlay47ExternalReloc((M2C_UNK *)0xE, NULL);
                    } else {
                        *(s32 *)0x30B = 1;
                        *NULL = 0;
                    }
                }
            }
            if ((overlay47ExternalReloc(sp278) & 0x9000) && (M2C_FIELD(var_s0, s8 *, 0x2C) == 0) && (M2C_FIELD(var_s0, M2C_UNK **, 0x24) != NULL) && (*(s32 *)0x324 == 0)) {
                if (sp258 != 0) {
                    if (*(s32 *)0x338 == 0) {
                        *(s32 *)0x338 = 1;
                        overlay47ExternalReloc((M2C_UNK *)0xC, NULL);
                    } else {
                        sp250 = 1;
                        temp_a0 = M2C_FIELD(var_s0, M2C_UNK **, 0x30);
                        if (temp_a0 != NULL) {
                            overlay47ExternalReloc(temp_a0);
                        }
                        overlay47ExternalReloc((M2C_UNK *) M2C_FIELD((M2C_FIELD(M2C_FIELD(var_s0, s16 *, 0x28), s8 *, 0x524) * 2), u16 *, 0x4B4), var_s0 + 0x30);
                    }
                } else if (M2C_FIELD(var_s0, s8 *, 0x2A) == 0) {
                    temp_a0_2 = M2C_FIELD(var_s0, M2C_UNK **, 0x30);
                    M2C_FIELD(var_s0, s8 *, 0x2A) = 1;
                    if (temp_a0_2 != NULL) {
                        overlay47ExternalReloc(temp_a0_2);
                    }
                    overlay47ExternalReloc((M2C_UNK *) M2C_FIELD((M2C_FIELD(M2C_FIELD(var_s0, s16 *, 0x28), s8 *, 0x524) * 2), u16 *, 0x48C), var_s0 + 0x30);
                }
                overlay47ExternalReloc(M2C_FIELD(var_s0, M2C_UNK **, 0x24), (M2C_UNK *)2, (M2C_UNK *)-1, NULL);
            } else if ((overlay47ExternalReloc(sp278) & 0x4000) && (M2C_FIELD(var_s0, s8 *, 0x2C) == 0)) {
                temp_s2_2 = M2C_FIELD(var_s0, M2C_UNK **, 0x24);
                if ((temp_s2_2 != NULL) && (*(s32 *)0x324 == 0) && (sp250 == 0)) {
                    if (M2C_FIELD(var_s0, s8 *, 0x2A) != 0) {
                        M2C_FIELD(var_s0, s8 *, 0x2A) = 0;
                        sp258 = 0;
                        overlay47ExternalReloc(temp_s2_2, NULL, (M2C_UNK *)-1, NULL);
                        temp_a0_3 = M2C_FIELD(var_s0, M2C_UNK **, 0x30);
                        if (temp_a0_3 != NULL) {
                            overlay47ExternalReloc(temp_a0_3);
                        }
                        overlay47ExternalReloc((M2C_UNK *) M2C_FIELD((M2C_FIELD(M2C_FIELD(var_s0, s16 *, 0x28), s8 *, 0x524) * 2), u16 *, 0x4A0), var_s0 + 0x30);
                    } else if (*NULL >= 2) {
                        M2C_FIELD(var_s0, s8 *, 0x2B) = 0;
                        *(&D_300 + M2C_FIELD(var_s0, s16 *, 0x28)) = 0;
                        temp_s2_3 = M2C_FIELD(var_s0, M2C_UNK **, 0x24);
                        if (temp_s2_3 != NULL) {
                            M2C_FIELD(var_s0, s8 *, 0x2C) = 1;
                            overlay47ExternalReloc(temp_s2_3, (M2C_UNK *)5, (M2C_UNK *)-1, NULL);
                            overlay47ExternalReloc((M2C_UNK *)0x18, NULL);
                        }
                        *NULL = (s32) (*NULL - 1);
                    } else {
                        sp214 = 1;
                    }
                }
            }
        }
        if ((M2C_FIELD(var_s0, s8 *, 0x2B) != 0) && (M2C_FIELD(var_s0, M2C_UNK **, 0x24) == NULL)) {
            func_overlay_047_F0002D10_1893B28(var_s0);
        }
        if (M2C_FIELD(var_s0, M2C_UNK **, 0x24) != NULL) {
            temp_s1 = arg0 * 0x2710;
            var_s4_2 = 0;
            overlay47ExternalReloc(M2C_FIELD(M2C_FIELD(var_s0, M2C_UNK **, 0x24), M2C_UNK **, 0xC), M2C_FIELD(M2C_FIELD(var_s0, M2C_UNK **, 0x24), M2C_UNK **, 0x10), M2C_FIELD(M2C_FIELD(var_s0, M2C_UNK **, 0x24), M2C_UNK **, 0x14), var_s0 + 0x18, var_s0 + 0x1C, 1e-45f);
            if ((M2C_FIELD(var_s0, s8 *, 0x2C) == 0) && (M2C_FIELD(var_s0, s8 *, 0x2B) == 0)) {
                var_f14 = temp_f26;
                M2C_FIELD(var_s0, f32 *, 0xC) = (f32) M2C_FIELD(&D_2F0, f32 *, 0);
                M2C_FIELD(var_s0, f32 *, 0x10) = (f32) M2C_FIELD(&D_2F0, f32 *, 4);
                M2C_FIELD(var_s0, f32 *, 0x14) = (f32) M2C_FIELD(&D_2F0, f32 *, 8);
                M2C_FIELD(var_s0, s16 *, 0x22) = (s16) M2C_FIELD(&D_2F0, s16 *, 0xC);
            } else {
                var_f14 = 0.125f;
                if (M2C_FIELD(var_s0, s8 *, 0x2A) != 0) {
                    M2C_FIELD(var_s0, f32 *, 0xC) = (f32) *(&D_280 + (M2C_FIELD((&D_3CC + (D_30A * 4) + var_s6), s8 *, -4) * 0x10));
                    M2C_FIELD(var_s0, f32 *, 0x10) = (f32) M2C_FIELD((&D_280 + (M2C_FIELD((&D_3CC + (D_30A * 4) + var_s6), s8 *, -4) * 0x10)), f32 *, 4);
                    M2C_FIELD(var_s0, f32 *, 0x14) = (f32) M2C_FIELD((&D_280 + (M2C_FIELD((&D_3CC + (D_30A * 4) + var_s6), s8 *, -4) * 0x10)), f32 *, 8);
                    temp_t7 = M2C_FIELD((&D_3CC + (D_30A * 4) + var_s6), s8 *, -4);
                    var_s6 += 1;
                    M2C_FIELD(var_s0, s16 *, 0x22) = (s16) M2C_FIELD((&D_280 + (temp_t7 * 0x10)), s16 *, 0xC);
                } else {
                    M2C_FIELD(var_s0, f32 *, 0xC) = (f32) *(&D_210 + (M2C_FIELD(((D_30A * 4) + var_s6), s8 *, 0x3C8) * 0x10));
                    M2C_FIELD(var_s0, f32 *, 0x10) = (f32) M2C_FIELD((&D_210 + (M2C_FIELD(((D_30A * 4) + var_s6), s8 *, 0x3C8) * 0x10)), f32 *, 4);
                    M2C_FIELD(var_s0, f32 *, 0x14) = (f32) M2C_FIELD((&D_210 + (M2C_FIELD(((D_30A * 4) + var_s6), s8 *, 0x3C8) * 0x10)), f32 *, 8);
                    temp_t6_2 = M2C_FIELD(((D_30A * 4) + var_s6), s8 *, 0x3C8);
                    var_s6 += 1;
                    M2C_FIELD(var_s0, s16 *, 0x22) = (s16) M2C_FIELD((&D_210 + (temp_t6_2 * 0x10)), s16 *, 0xC);
                }
            }
            if ((arg0 > 0) && (!(arg0 & 1) || (temp_f0 = M2C_FIELD(var_s0, f32 *, 0), temp_f2 = M2C_FIELD(var_s0, f32 *, 4), temp_f12 = M2C_FIELD(var_s0, f32 *, 8), temp_v0_2 = M2C_FIELD(var_s0, s16 *, 0x20), var_s4_2 = 1, M2C_FIELD(var_s0, f32 *, 0) = (f32) (temp_f0 + ((M2C_FIELD(var_s0, f32 *, 0xC) - temp_f0) * var_f14)), M2C_FIELD(var_s0, f32 *, 4) = (f32) (temp_f2 + ((M2C_FIELD(var_s0, f32 *, 0x10) - temp_f2) * var_f14)), M2C_FIELD(var_s0, f32 *, 8) = (f32) (temp_f12 + ((M2C_FIELD(var_s0, f32 *, 0x14) - temp_f12) * var_f14)), M2C_FIELD(var_s0, s16 *, 0x20) = (s16) (s32) ((f32) temp_v0_2 + ((f32) (M2C_FIELD(var_s0, s16 *, 0x22) - temp_v0_2) * var_f14)), M2C_FIELD(M2C_FIELD(var_s0, M2C_UNK **, 0x24), f32 *, 0xC) = (f32) M2C_FIELD(var_s0, f32 *, 0), M2C_FIELD(M2C_FIELD(var_s0, M2C_UNK **, 0x24), f32 *, 0x10) = (f32) M2C_FIELD(var_s0, f32 *, 4), M2C_FIELD(M2C_FIELD(var_s0, M2C_UNK **, 0x24), f32 *, 0x14) = (f32) M2C_FIELD(var_s0, f32 *, 8), M2C_FIELD(M2C_FIELD(var_s0, M2C_UNK **, 0x24), s16 *, 0) = (s16) M2C_FIELD(var_s0, s16 *, 0x20), (arg0 != 1)))) {
                var_s4_3 = var_s4_2 + 2;
                if (var_s4_3 != arg0) {
                    do {
                        temp_f12_2 = M2C_FIELD(var_s0, f32 *, 4);
                        temp_f8 = M2C_FIELD(var_s0, f32 *, 8);
                        temp_v0_3 = M2C_FIELD(var_s0, s16 *, 0x20);
                        M2C_FIELD(var_s0, f32 *, 0) = (f32) (M2C_FIELD(var_s0, f32 *, 0) + ((M2C_FIELD(var_s0, f32 *, 0xC) - M2C_FIELD(var_s0, f32 *, 0)) * var_f14));
                        var_s4_3 += 2;
                        M2C_FIELD(var_s0, f32 *, 4) = (f32) (temp_f12_2 + ((M2C_FIELD(var_s0, f32 *, 0x10) - temp_f12_2) * var_f14));
                        M2C_FIELD(var_s0, f32 *, 8) = (f32) (temp_f8 + ((M2C_FIELD(var_s0, f32 *, 0x14) - temp_f8) * var_f14));
                        M2C_FIELD(var_s0, s16 *, 0x20) = (s16) (s32) ((f32) temp_v0_3 + ((f32) (M2C_FIELD(var_s0, s16 *, 0x22) - temp_v0_3) * var_f14));
                        M2C_FIELD(M2C_FIELD(var_s0, M2C_UNK **, 0x24), f32 *, 0xC) = (f32) M2C_FIELD(var_s0, f32 *, 0);
                        M2C_FIELD(M2C_FIELD(var_s0, M2C_UNK **, 0x24), f32 *, 0x10) = (f32) M2C_FIELD(var_s0, f32 *, 4);
                        M2C_FIELD(M2C_FIELD(var_s0, M2C_UNK **, 0x24), f32 *, 0x14) = (f32) M2C_FIELD(var_s0, f32 *, 8);
                        M2C_FIELD(M2C_FIELD(var_s0, M2C_UNK **, 0x24), s16 *, 0) = (s16) M2C_FIELD(var_s0, s16 *, 0x20);
                        temp_f16 = M2C_FIELD(var_s0, f32 *, 0);
                        temp_f4 = M2C_FIELD(var_s0, f32 *, 4);
                        temp_v0_4 = M2C_FIELD(var_s0, s16 *, 0x20);
                        temp_f10 = M2C_FIELD(var_s0, f32 *, 8);
                        M2C_FIELD(var_s0, f32 *, 0) = (f32) (temp_f16 + ((M2C_FIELD(var_s0, f32 *, 0xC) - temp_f16) * var_f14));
                        M2C_FIELD(var_s0, f32 *, 4) = (f32) (temp_f4 + ((M2C_FIELD(var_s0, f32 *, 0x10) - temp_f4) * var_f14));
                        M2C_FIELD(var_s0, f32 *, 8) = (f32) (temp_f10 + ((M2C_FIELD(var_s0, f32 *, 0x14) - temp_f10) * var_f14));
                        M2C_FIELD(var_s0, s16 *, 0x20) = (s16) (s32) ((f32) temp_v0_4 + ((f32) (M2C_FIELD(var_s0, s16 *, 0x22) - temp_v0_4) * var_f14));
                        M2C_FIELD(M2C_FIELD(var_s0, M2C_UNK **, 0x24), f32 *, 0xC) = (f32) M2C_FIELD(var_s0, f32 *, 0);
                        M2C_FIELD(M2C_FIELD(var_s0, M2C_UNK **, 0x24), f32 *, 0x10) = (f32) M2C_FIELD(var_s0, f32 *, 4);
                        M2C_FIELD(M2C_FIELD(var_s0, M2C_UNK **, 0x24), f32 *, 0x14) = (f32) M2C_FIELD(var_s0, f32 *, 8);
                        M2C_FIELD(M2C_FIELD(var_s0, M2C_UNK **, 0x24), s16 *, 0) = (s16) M2C_FIELD(var_s0, s16 *, 0x20);
                    } while (var_s4_3 != arg0);
                }
                temp_f12_3 = M2C_FIELD(var_s0, f32 *, 4);
                temp_v0_5 = M2C_FIELD(var_s0, s16 *, 0x20);
                temp_f8_2 = M2C_FIELD(var_s0, f32 *, 8);
                M2C_FIELD(var_s0, f32 *, 0) = (f32) (M2C_FIELD(var_s0, f32 *, 0) + ((M2C_FIELD(var_s0, f32 *, 0xC) - M2C_FIELD(var_s0, f32 *, 0)) * var_f14));
                M2C_FIELD(var_s0, f32 *, 4) = (f32) (temp_f12_3 + ((M2C_FIELD(var_s0, f32 *, 0x10) - temp_f12_3) * var_f14));
                M2C_FIELD(var_s0, f32 *, 8) = (f32) (temp_f8_2 + ((M2C_FIELD(var_s0, f32 *, 0x14) - temp_f8_2) * var_f14));
                M2C_FIELD(var_s0, s16 *, 0x20) = (s16) (s32) ((f32) temp_v0_5 + ((f32) (M2C_FIELD(var_s0, s16 *, 0x22) - temp_v0_5) * var_f14));
                M2C_FIELD(M2C_FIELD(var_s0, M2C_UNK **, 0x24), f32 *, 0xC) = (f32) M2C_FIELD(var_s0, f32 *, 0);
                M2C_FIELD(M2C_FIELD(var_s0, M2C_UNK **, 0x24), f32 *, 0x10) = (f32) M2C_FIELD(var_s0, f32 *, 4);
                M2C_FIELD(M2C_FIELD(var_s0, M2C_UNK **, 0x24), f32 *, 0x14) = (f32) M2C_FIELD(var_s0, f32 *, 8);
                M2C_FIELD(M2C_FIELD(var_s0, M2C_UNK **, 0x24), s16 *, 0) = (s16) M2C_FIELD(var_s0, s16 *, 0x20);
                temp_f16_2 = M2C_FIELD(var_s0, f32 *, 0);
                temp_v0_6 = M2C_FIELD(var_s0, s16 *, 0x20);
                temp_f4_2 = M2C_FIELD(var_s0, f32 *, 4);
                temp_f10_2 = M2C_FIELD(var_s0, f32 *, 8);
                M2C_FIELD(var_s0, f32 *, 0) = (f32) (temp_f16_2 + ((M2C_FIELD(var_s0, f32 *, 0xC) - temp_f16_2) * var_f14));
                M2C_FIELD(var_s0, f32 *, 4) = (f32) (temp_f4_2 + ((M2C_FIELD(var_s0, f32 *, 0x10) - temp_f4_2) * var_f14));
                M2C_FIELD(var_s0, f32 *, 8) = (f32) (temp_f10_2 + ((M2C_FIELD(var_s0, f32 *, 0x14) - temp_f10_2) * var_f14));
                M2C_FIELD(var_s0, s16 *, 0x20) = (s16) (s32) ((f32) temp_v0_6 + ((f32) (M2C_FIELD(var_s0, s16 *, 0x22) - temp_v0_6) * var_f14));
                M2C_FIELD(M2C_FIELD(var_s0, M2C_UNK **, 0x24), f32 *, 0xC) = (f32) M2C_FIELD(var_s0, f32 *, 0);
                M2C_FIELD(M2C_FIELD(var_s0, M2C_UNK **, 0x24), f32 *, 0x10) = (f32) M2C_FIELD(var_s0, f32 *, 4);
                M2C_FIELD(M2C_FIELD(var_s0, M2C_UNK **, 0x24), f32 *, 0x14) = (f32) M2C_FIELD(var_s0, f32 *, 8);
                M2C_FIELD(M2C_FIELD(var_s0, M2C_UNK **, 0x24), s16 *, 0) = (s16) M2C_FIELD(var_s0, s16 *, 0x20);
            }
            if (overlay47ExternalReloc((M2C_UNK *)(u32)M2C_FIELD(var_s0, f32 *, 0), (M2C_UNK *)(u32)M2C_FIELD(var_s0, f32 *, 8), (M2C_UNK *)0x1000, &sp12C) != 0) {
                M2C_FIELD(var_s0, f32 *, 4) = (f32)*sp12C;
            }
            temp_a0_4 = M2C_FIELD(var_s0, M2C_UNK **, 0x24);
            temp_f20 = M2C_FIELD(temp_a0_4, f32 *, 0x28);
            overlay47ExternalReloc(temp_a0_4, M2C_FIELD((M2C_FIELD(temp_a0_4, s8 *, 0x3B) * 4), M2C_UNK **, 0x3F0), (M2C_UNK *)(u32)temp_f24);
            temp_v1 = M2C_FIELD(M2C_FIELD(var_s0, M2C_UNK **, 0x24), void **, 0x64);
            M2C_FIELD(temp_v1, s16 *, 0xA) = (s16) (M2C_FIELD(temp_v1, s16 *, 0xA) + temp_s1);
            M2C_FIELD(temp_v1, s16 *, 8) = (s16) (M2C_FIELD(temp_v1, s16 *, 8) + temp_s1);
            M2C_FIELD(temp_v1, s16 *, 0xE) = (s16) (M2C_FIELD(temp_v1, s16 *, 0xE) + temp_s1);
            M2C_FIELD(temp_v1, s16 *, 0xC) = (s16) (M2C_FIELD(temp_v1, s16 *, 0xC) + temp_s1);
            if ((M2C_FIELD(var_s0, s8 *, 0x2A) == 0) && (M2C_FIELD(var_s0, s8 *, 0x2C) == 0) && (M2C_FIELD(var_s0, s8 *, 0x2B) != 0) && (sp250 == 0)) {
                temp_v0_7 = *sp278;
                if ((temp_v0_7 < -0x10) && (*(s32 *)0x324 == 0)) {
                    *(&D_300 + M2C_FIELD(var_s0, s16 *, 0x28)) = 0;
                    M2C_FIELD(var_s0, s16 *, 0x28) = (s16) (M2C_FIELD(var_s0, s16 *, 0x28) - 1);
                    if (M2C_FIELD(var_s0, s16 *, 0x28) < 0) {
                        M2C_FIELD(var_s0, s16 *, 0x28) = 9;
                    }
                    var_v1_2 = &D_300 + M2C_FIELD(var_s0, s16 *, 0x28);
                    if (*var_v1_2 != 0) {
                        do {
                            M2C_FIELD(var_s0, s16 *, 0x28) = (s16) (M2C_FIELD(var_s0, s16 *, 0x28) - 1);
                            if (M2C_FIELD(var_s0, s16 *, 0x28) < 0) {
                                M2C_FIELD(var_s0, s16 *, 0x28) = 9;
                            }
                            var_v1_2 = &D_300 + M2C_FIELD(var_s0, s16 *, 0x28);
                        } while (*var_v1_2 != 0);
                    }
                    *var_v1_2 = 1;
                    *(s32 *)0x30B = 1;
                    overlay47ExternalReloc(M2C_FIELD(var_s0, M2C_UNK **, 0x24));
                    M2C_FIELD(var_s0, M2C_UNK **, 0x24) = NULL;
                    func_overlay_047_F0002D10_1893B28(var_s0);
                } else if ((temp_v0_7 >= 0x11) && (*(s32 *)0x324 == 0)) {
                    *(&D_300 + M2C_FIELD(var_s0, s16 *, 0x28)) = 0;
                    M2C_FIELD(var_s0, s16 *, 0x28) = (s16) (M2C_FIELD(var_s0, s16 *, 0x28) + 1);
                    if (M2C_FIELD(var_s0, s16 *, 0x28) >= 0xA) {
                        M2C_FIELD(var_s0, s16 *, 0x28) = 0;
                    }
                    var_v1_3 = &D_300 + M2C_FIELD(var_s0, s16 *, 0x28);
                    if (*var_v1_3 != 0) {
                        do {
                            M2C_FIELD(var_s0, s16 *, 0x28) = (s16) (M2C_FIELD(var_s0, s16 *, 0x28) + 1);
                            if (M2C_FIELD(var_s0, s16 *, 0x28) >= 0xA) {
                                M2C_FIELD(var_s0, s16 *, 0x28) = 0;
                            }
                            var_v1_3 = &D_300 + M2C_FIELD(var_s0, s16 *, 0x28);
                        } while (*var_v1_3 != 0);
                    }
                    *var_v1_3 = 1;
                    *(s32 *)0x30B = 1;
                    overlay47ExternalReloc(M2C_FIELD(var_s0, M2C_UNK **, 0x24));
                    M2C_FIELD(var_s0, M2C_UNK **, 0x24) = NULL;
                    func_overlay_047_F0002D10_1893B28(var_s0);
                }
            }
            temp_s2_4 = M2C_FIELD(var_s0, M2C_UNK **, 0x24);
            if (temp_s2_4 != NULL) {
                M2C_FIELD(temp_s2_4, s32 *, 0x80) = 0;
                var_s2 = M2C_FIELD(var_s0, M2C_UNK **, 0x24);
                temp_t9 = (u8) M2C_FIELD(var_s2, s8 *, 0x3B);
                switch (temp_t9) {
                case 1:
                    var_f0 = M2C_FIELD(var_s2, f32 *, 0x28);
                    if ((temp_f22 <= var_f0) && (var_f0 < *(f32 *)0x24)) {
                        M2C_FIELD(var_s2, s32 *, 0x80) = 0xF;
                        var_s2 = M2C_FIELD(var_s0, M2C_UNK **, 0x24);
                        var_f0 = M2C_FIELD(var_s2, f32 *, 0x28);
                    }
                    if ((temp_f20 < temp_f22) && (temp_f22 <= var_f0)) {
                        overlay47ExternalReloc((M2C_UNK *)0x1A, NULL);
                        var_s2 = M2C_FIELD(var_s0, M2C_UNK **, 0x24);
                        var_f0 = M2C_FIELD(var_s2, f32 *, 0x28);
                    }
                    if (var_f0 == 1.0f) {
                        overlay47ExternalReloc(var_s2, NULL, (M2C_UNK *)-1, NULL);
                        M2C_FIELD(var_s0, s16 *, 0x2E) = overlay47ExternalReloc((M2C_UNK *)0x1E, (M2C_UNK *)0x12C);
block_132:
                        var_s2 = M2C_FIELD(var_s0, M2C_UNK **, 0x24);
                    }
                    break;
                case 3:
                case 4:
                    if (M2C_FIELD(var_s2, f32 *, 0x28) == 1.0f) {
                        overlay47ExternalReloc(var_s2, NULL, NULL, NULL);
                        M2C_FIELD(var_s0, s16 *, 0x2E) = overlay47ExternalReloc((M2C_UNK *)0x1E, (M2C_UNK *)0x12C);
                        goto block_132;
                    }
                    break;
                case 0:
                    temp_v0_8 = M2C_FIELD(var_s0, s16 *, 0x2E);
                    if (temp_v0_8 > 0) {
                        M2C_FIELD(var_s0, s16 *, 0x2E) = (s16) (temp_v0_8 - arg0);
                        goto block_132;
                    }
                    if (M2C_FIELD(var_s2, f32 *, 0x28) < *(f32 *)0x28) {
                        overlay47ExternalReloc(M2C_FIELD(var_s0, M2C_UNK **, 0x24), overlay47ExternalReloc((M2C_UNK *)3, (M2C_UNK *)4), (M2C_UNK *)-1, NULL);
                        goto block_132;
                    }
                    break;
                case 5:
                    M2C_FIELD(var_s2, s32 *, 0x80) = 0xF;
                    var_s2 = M2C_FIELD(var_s0, M2C_UNK **, 0x24);
                    if ((*(f32 *)0x2C < M2C_FIELD(var_s2, f32 *, 0x28)) && (M2C_FIELD(var_s0, s8 *, 0x2C) != 0)) {
                        M2C_FIELD(var_s0, s8 *, 0x2C) = 0;
                        var_s6 -= 1;
                        *(s32 *)0x30A = (s8) (*(s8 *)0x30A - 1);
                        goto block_132;
                    }
                    break;
                case 2:
                    if (M2C_FIELD(var_s2, f32 *, 0x28) == 1.0f) {
                        overlay47ExternalReloc(var_s2, NULL, (M2C_UNK *)-1, NULL);
                        M2C_FIELD(var_s0, s16 *, 0x2E) = overlay47ExternalReloc((M2C_UNK *)0x1E, (M2C_UNK *)0x12C);
                        goto block_132;
                    }
                    break;
                }
                if ((M2C_FIELD(var_s0, s8 *, 0x2C) == 0) && (M2C_FIELD(var_s0, s8 *, 0x2B) == 0)) {
                    temp_f0_2 = M2C_FIELD(var_s0, f32 *, 0) - M2C_FIELD(&D_2F0, f32 *, 0);
                    temp_f2_2 = M2C_FIELD(var_s0, f32 *, 8) - M2C_FIELD(&D_2F0, f32 *, 8);
                    if (((temp_f0_2 * temp_f0_2) + (temp_f2_2 * temp_f2_2)) < 1000.0f) {
                        overlay47ExternalReloc(var_s2);
                        M2C_FIELD(var_s0, M2C_UNK **, 0x24) = NULL;
                        var_s2 = NULL;
                    }
                }
                if (var_s2 != NULL) {
                    overlay47ExternalReloc(var_s2, (M2C_UNK *) arg0);
                }
            }
        }
        var_s0 += 0x34;
        temp_t6 = sp278 + 1;
        sp278 = temp_t6;
    } while ((s32) temp_t6 < 4);
    temp_t6_3 = D_550 + (D_554 * arg0);
    D_550 = temp_t6_3;
    if (temp_t6_3 < 0) {
        D_550 = -temp_t6_3;
        D_554 = -D_554;
    } else if (temp_t6_3 >= 0x100) {
        D_550 = 0x1FE - temp_t6_3;
        D_554 = -D_554;
    }
    if (sp21C != 0) {
        overlay47ExternalReloc(*(M2C_UNK **)0x318, (M2C_UNK *)0xA0, (M2C_UNK *)0xA0, (M2C_UNK *)0x104);
        overlay47ExternalReloc(D_31C, (M2C_UNK *)0x78, (M2C_UNK *)0xBE, (M2C_UNK *)0x104);
        overlay47ExternalReloc(D_320, (M2C_UNK *)0xC8, (M2C_UNK *)0xBE, (M2C_UNK *)0x104);
        if ((u8) *NULL != 0) {
            overlay47ExternalReloc(D_31C, (M2C_UNK *) D_550);
            overlay47ExternalReloc(D_320, NULL);
        } else {
            overlay47ExternalReloc(D_31C, NULL);
            overlay47ExternalReloc(D_320, (M2C_UNK *) D_550);
        }
    } else {
        overlay47ExternalReloc(*(s32 *)0x318, (M2C_UNK *)0xA0, (M2C_UNK *)-0x28, (M2C_UNK *)0x104);
        overlay47ExternalReloc(D_31C, (M2C_UNK *)0x78, (M2C_UNK *)0x104, (M2C_UNK *)0x104);
        overlay47ExternalReloc(D_320, (M2C_UNK *)0xC8, (M2C_UNK *)0x104, (M2C_UNK *)0x104);
    }
    sp1F8 = M2C_BITWISE(f32, overlay47ExternalReloc());
    overlay47ExternalReloc((M2C_UNK *)0x42500000, (M2C_UNK *)1);
    overlay47ExternalReloc(NULL, NULL);
    overlay47ExternalReloc(NULL, NULL);
    overlay47ExternalReloc(NULL, NULL, (M2C_UNK *)0x10, NULL);
    temp_v0_9 = *NULL;
    *NULL = temp_v0_9 + 8;
    M2C_FIELD(temp_v0_9, M2C_UNK **, 4) = (M2C_UNK *)-1;
    M2C_FIELD(temp_v0_9, M2C_UNK **, 0) = (M2C_UNK *)0xFA000000;
    temp_v0_10 = *NULL;
    *NULL = temp_v0_10 + 8;
    M2C_FIELD(temp_v0_10, M2C_UNK **, 0) = (M2C_UNK *) ((((((s32) &D_80000228 & 6) | 0x30) & 0xFF) << 0x10) | 0x04000000 | 0x44);
    M2C_FIELD(temp_v0_10, M2C_UNK **, 4) = &D_80000228;
    temp_v0_11 = *NULL;
    *NULL = temp_v0_11 + 8;
    M2C_FIELD(temp_v0_11, M2C_UNK **, 4) = &D_80000268;
    M2C_FIELD(temp_v0_11, M2C_UNK **, 0) = (M2C_UNK *)0x05300040;
    temp_v0_12 = *NULL;
    *NULL = temp_v0_12 + 8;
    M2C_FIELD(temp_v0_12, M2C_UNK **, 4) = NULL;
    M2C_FIELD(temp_v0_12, M2C_UNK **, 0) = (M2C_UNK *)0xE7000000;
    if ((sp258 != 0) && (*(s32 *)0x338 != 0)) {
        overlay47ExternalReloc(*(M2C_UNK **)0x314, (M2C_UNK *)0xA0, (M2C_UNK *)0xB2, (M2C_UNK *)0x104);
    } else {
        overlay47ExternalReloc(*(s32 *)0x314, (M2C_UNK *)0xA0, (M2C_UNK *)0x110, (M2C_UNK *)0x104);
    }
    sp278 = NULL;
    if ((s8) *NULL > 0) {
        sp8C = &D_300;
        sp88 = &D_80000198;
        temp_f26_2 = *(f32 *)0x30;
        sp84 = (((((s32) &D_80000198 & 6) | 0x20) & 0xFF) << 0x10) | 0x04000000 | 0x30;
        sp80 = &D_800001C0;
        do {
            var_s7 = 4;
            var_fp_2 = 0;
            var_s1 = -1;
            var_t0 = &D_1C0;
            var_a1_2 = NULL;
            var_s4_4 = 0;
loop_156:
            if (M2C_FIELD(var_s3, f32 *, 0x28) == (f32) M2C_FIELD(var_a1_2, s16 *, 0x28)) {
                temp_v0_14 = var_s4_4 * 4;
                if (M2C_FIELD(var_a1_2, s8 *, 0x2B) != 0) {
                    var_s7 = var_s4_4;
                    var_s1_2 = 0;
                    temp_v1_2 = temp_v0_14 + &D_328;
                    if (M2C_FIELD(var_a1_2, s8 *, 0x2A) == 0) {
                        var_fp_2 = 1;
                    }
                    temp_v0_15 = arg0 & 3;
                    if (arg0 > 0) {
                        if (temp_v0_15 != 0) {
                            var_s1_2 = 1;
                            if (temp_v0_15 != 1) {
                                do {
                                    temp_v0_16 = *temp_v1_2;
                                    var_s1_2 += 1;
                                    *temp_v1_2 = temp_v0_16 + ((s32) ((((s32) M2C_FIELD(var_s3, f32 *, 0xC) + 0xA0) * 0x10) - temp_v0_16) >> 2);
                                } while (temp_v0_15 != var_s1_2);
                            }
                            temp_v0_17 = *temp_v1_2;
                            *temp_v1_2 = temp_v0_17 + ((s32) ((((s32) M2C_FIELD(var_s3, f32 *, 0xC) + 0xA0) * 0x10) - temp_v0_17) >> 2);
                            if (var_s1_2 != arg0) {
                                goto block_165;
                            }
                        } else {
block_165:
                            var_s1_3 = var_s1_2 + 4;
                            if (var_s1_3 != arg0) {
                                do {
                                    temp_v0_18 = *temp_v1_2;
                                    var_s1_3 += 4;
                                    temp_t7_2 = temp_v0_18 + ((s32) ((((s32) M2C_FIELD(var_s3, f32 *, 0xC) + 0xA0) * 0x10) - temp_v0_18) >> 2);
                                    *temp_v1_2 = temp_t7_2;
                                    temp_t9_2 = temp_t7_2 + ((s32) ((((s32) M2C_FIELD(var_s3, f32 *, 0xC) + 0xA0) * 0x10) - temp_t7_2) >> 2);
                                    *temp_v1_2 = temp_t9_2;
                                    temp_t8 = temp_t9_2 + ((s32) ((((s32) M2C_FIELD(var_s3, f32 *, 0xC) + 0xA0) * 0x10) - temp_t9_2) >> 2);
                                    *temp_v1_2 = temp_t8;
                                    *temp_v1_2 = temp_t8 + ((s32) ((((s32) M2C_FIELD(var_s3, f32 *, 0xC) + 0xA0) * 0x10) - temp_t8) >> 2);
                                } while (var_s1_3 != arg0);
                            }
                            temp_v0_19 = *temp_v1_2;
                            temp_t7_3 = temp_v0_19 + ((s32) ((((s32) M2C_FIELD(var_s3, f32 *, 0xC) + 0xA0) * 0x10) - temp_v0_19) >> 2);
                            *temp_v1_2 = temp_t7_3;
                            temp_t9_3 = temp_t7_3 + ((s32) ((((s32) M2C_FIELD(var_s3, f32 *, 0xC) + 0xA0) * 0x10) - temp_t7_3) >> 2);
                            *temp_v1_2 = temp_t9_3;
                            temp_t8_2 = temp_t9_3 + ((s32) ((((s32) M2C_FIELD(var_s3, f32 *, 0xC) + 0xA0) * 0x10) - temp_t9_3) >> 2);
                            *temp_v1_2 = temp_t8_2;
                            *temp_v1_2 = temp_t8_2 + ((s32) ((((s32) M2C_FIELD(var_s3, f32 *, 0xC) + 0xA0) * 0x10) - temp_t8_2) >> 2);
                        }
                    }
                    M2C_FIELD(var_t0, M2C_UNK **, 0) = M2C_FIELD(temp_v0_14, M2C_UNK **, 0x34);
                    M2C_FIELD(var_t0, s16 *, 0xC) = (s16) (0x138 - ((s32) *temp_v1_2 >> 4));
                    var_t0 += 0x10;
                    var_s1 = var_s4_4;
                }
            }
            var_s4_4 += 1;
            var_a1_2 += 0x34;
            if (var_s4_4 < 4) {
                goto loop_156;
            }
            *var_t0 = NULL;
            overlay47ExternalReloc(NULL, &D_1C0, (M2C_UNK *)0x43A00000, NULL, (void *)(u32)1.0f, 1.0f, -2, 0x1003);
            overlay47ExternalReloc(&sp1B8);
            overlay47ExternalReloc((M2C_UNK *)(u32)M2C_FIELD(var_s3, f32 *, 0xC), M2C_FIELD(var_s3, M2C_UNK **, 0x10), NULL, &sp1B8);
            temp_f12_4 = M2C_FIELD(var_s3, f32 *, 8) / temp_f26_2;
            overlay47ExternalReloc((M2C_UNK *)(u32)temp_f12_4, (M2C_UNK *)(u32)temp_f12_4, (M2C_UNK *)(u32)temp_f12_4, &sp1B8);
            overlay47ExternalReloc((M2C_UNK *) M2C_FIELD(var_s3, s16 *, 4), &sp1B8);
            overlay47ExternalReloc(&sp178);
            overlay47ExternalReloc(&sp1B8, &sp178, &sp138);
            overlay47ExternalReloc(&sp138, *NULL);
            temp_v0_20 = *NULL;
            temp_s0 = *NULL;
            *NULL = temp_v0_20 + 8;
            M2C_FIELD(temp_v0_20, M2C_UNK **, 0) = (M2C_UNK *)0x01000040;
            M2C_FIELD(temp_v0_20, M2C_UNK **, 4) = (M2C_UNK *) (*NULL + 0x80000000);
            temp_v0_21 = *NULL;
            *NULL += 0x40;
            temp_v1_3 = M2C_FIELD((var_s7 * 4), s32 *, 0x3DC);
            *NULL = temp_v0_21 + 8;
            M2C_FIELD(temp_v0_21, M2C_UNK **, 0) = (M2C_UNK *)0x06000000;
            temp_a3 = temp_v1_3 >> 0x10;
            M2C_FIELD(temp_v0_21, M2C_UNK **, 4) = sp8C;
            temp_v0_22 = *NULL;
            temp_a2 = temp_v1_3 >> 0x18;
            *NULL = temp_v0_22 + 8;
            M2C_FIELD(temp_v0_22, M2C_UNK **, 0) = (M2C_UNK *)0xFA000000;
            M2C_FIELD(temp_v0_22, M2C_UNK **, 4) = (M2C_UNK *) ((temp_a2 << 0x18) | ((temp_a3 & 0xFF) << 0x10) | (((temp_v1_3 >> 8) & 0xFF) << 8) | 0xFF);
            temp_v0_23 = *NULL;
            *NULL = temp_v0_23 + 8;
            M2C_FIELD(temp_v0_23, M2C_UNK **, 4) = (M2C_UNK *)0xFFFDF6FB;
            M2C_FIELD(temp_v0_23, M2C_UNK **, 0) = (M2C_UNK *)0xFCFFFFFF;
            temp_v0_24 = *NULL;
            *NULL = temp_v0_24 + 8;
            M2C_FIELD(temp_v0_24, M2C_UNK **, 0) = (M2C_UNK *) sp84;
            M2C_FIELD(temp_v0_24, M2C_UNK **, 4) = sp88;
            temp_v0_25 = *NULL;
            *NULL = temp_v0_25 + 8;
            M2C_FIELD(temp_v0_25, M2C_UNK **, 0) = (M2C_UNK *)0x05100020;
            M2C_FIELD(temp_v0_25, M2C_UNK **, 4) = sp80;
            overlay47ExternalReloc(NULL, NULL, (M2C_UNK *) temp_a2, (M2C_UNK *) temp_a3);
            overlay47ExternalReloc(NULL);
            temp_v0_26 = *NULL;
            *NULL = temp_v0_26 + 8;
            M2C_FIELD(temp_v0_26, M2C_UNK **, 4) = (M2C_UNK *)-1;
            M2C_FIELD(temp_v0_26, M2C_UNK **, 0) = (M2C_UNK *)0xFA000000;
            overlay47ExternalReloc(NULL, NULL, NULL, (M2C_UNK *) var_s3, M2C_BITWISE(void *, *(s32 *)0x2C), 0.0f, 0xFF);
            if ((var_s1 != -1) && (var_fp_2 == 0)) {
                temp_f20_2 = M2C_FIELD(var_s3, f32 *, 0x28);
                M2C_FIELD(var_s3, f32 *, 0x28) = (f32) var_s1;
                overlay47ExternalReloc(NULL, NULL, NULL, (M2C_UNK *) var_s3, *(void **)0x1F0, 0.0f, 0xFF);
                M2C_FIELD(var_s3, f32 *, 0x28) = temp_f20_2;
            }
            temp_v0_27 = *NULL;
            *NULL = temp_v0_27 + 8;
            M2C_FIELD(temp_v0_27, M2C_UNK **, 0) = (M2C_UNK *)0x01000040;
            M2C_FIELD(temp_v0_27, M2C_UNK **, 4) = (M2C_UNK *) (temp_s0 + 0x80000000);
            if ((var_s1 != -1) && (M2C_FIELD((var_s1 * 0x34), s8 *, 0x2A) == 0)) {
                temp_v0_28 = *NULL;
                *NULL = temp_v0_28 + 8;
                M2C_FIELD(temp_v0_28, M2C_UNK **, 4) = (M2C_UNK *) &D_2A8;
                M2C_FIELD(temp_v0_28, M2C_UNK **, 0) = (M2C_UNK *)0x06000000;
                temp_v1_4 = M2C_FIELD((var_s1 * 4), s32 *, 0x3DC);
                temp_t7_4 = (temp_v1_4 >> 0x18) & 0xFF;
                temp_f0_3 = *(f32 *)0x540;
                temp_t9_4 = (temp_v1_4 >> 0x10) & 0xFF;
                temp_t8_3 = (temp_v1_4 >> 8) & 0xFF;
                temp_v0_29 = *NULL;
                *NULL = temp_v0_29 + 8;
                M2C_FIELD(temp_v0_29, M2C_UNK **, 0) = (M2C_UNK *)0xFA000000;
                M2C_FIELD(temp_v0_29, M2C_UNK **, 4) = (M2C_UNK *) (((s32) ((f32) temp_t7_4 + ((f32) (0xFF - temp_t7_4) * temp_f0_3)) << 0x18) | (((s32) ((f32) temp_t9_4 + ((f32) (0xFF - temp_t9_4) * temp_f0_3)) & 0xFF) << 0x10) | (((s32) ((f32) temp_t8_3 + ((f32) (0xFF - temp_t8_3) * temp_f0_3)) & 0xFF) << 8) | 0xFF);
                temp_v0_30 = *NULL;
                *NULL = temp_v0_30 + 8;
                M2C_FIELD(temp_v0_30, M2C_UNK **, 4) = (M2C_UNK *)0xFFFDF6FB;
                M2C_FIELD(temp_v0_30, M2C_UNK **, 0) = (M2C_UNK *)0xFCFFFFFF;
                temp_v0_31 = *NULL;
                *NULL = temp_v0_31 + 8;
                M2C_FIELD(temp_v0_31, M2C_UNK **, 0) = (M2C_UNK *) ((((((s32) &D_80000000 & 6) | 0x70) & 0xFF) << 0x10) | 0x04000000 | 0x94);
                M2C_FIELD(temp_v0_31, M2C_UNK **, 4) = (M2C_UNK *) &D_80000000;
                temp_v0_32 = *NULL;
                *NULL = temp_v0_32 + 8;
                M2C_FIELD(temp_v0_32, M2C_UNK **, 4) = (M2C_UNK *) &D_80000118;
                M2C_FIELD(temp_v0_32, M2C_UNK **, 0) = (M2C_UNK *)0x05710080;
                temp_v0_33 = *NULL;
                *NULL = temp_v0_33 + 8;
                M2C_FIELD(temp_v0_33, M2C_UNK **, 4) = (M2C_UNK *) &D_8000008C;
                M2C_FIELD(temp_v0_33, M2C_UNK **, 0) = (M2C_UNK *) ((((((s32) &D_8000008C & 6) | 0x70) & 0xFF) << 0x10) | 0x04000000 | 0x94);
                temp_v0_34 = *NULL;
                *NULL = temp_v0_34 + 8;
                M2C_FIELD(temp_v0_34, M2C_UNK **, 4) = (M2C_UNK *) &D_80000118;
                M2C_FIELD(temp_v0_34, M2C_UNK **, 0) = (M2C_UNK *)0x05710080;
            }
            var_s4_5 = 0;
            if (arg0 > 0) {
                temp_a1 = (s8 *)&D_474 + (s32)sp278;
                do {
                    temp_v1_5 = M2C_FIELD(var_s3, s16 *, 4);
                    if (temp_v1_5 < 0) {
                        var_a0 = (s32) (temp_v1_5 + 0x1000) / 5;
                    } else {
                        var_a0 = (s32) (0x1000 - temp_v1_5) / 5;
                    }
                    if (var_a0 < 0x33) {
                        var_a0 = 0x32;
                    }
                    if (*temp_a1 == 0) {
                        M2C_FIELD(var_s3, s16 *, 4) = (s16) (temp_v1_5 + var_a0);
                        if (var_s1 == -1) {
                            temp_v1_6 = M2C_FIELD(var_s3, s16 *, 4);
                            if ((temp_v1_6 >= 0) && ((temp_v1_6 - var_a0) <= 0)) {
                                M2C_FIELD(var_s3, s16 *, 4) = 0;
                            }
                        }
                        temp_v1_7 = M2C_FIELD(var_s3, s16 *, 4);
                        if (temp_v1_7 >= 0x1001) {
                            M2C_FIELD(var_s3, s16 *, 4) = (s16) (0x2000 - temp_v1_7);
                            *temp_a1 = 1;
                        }
                    } else {
                        temp_v0_35 = (s8 *)&D_480 + (s32)sp278;
                        if (temp_v1_5 < 0) {
                            *temp_v0_35 += 1;
                            if (*temp_v0_35 >= 0x65) {
                                *temp_v0_35 = 0x64;
                            }
                        } else {
                            *temp_v0_35 -= 1;
                            if (*temp_v0_35 < 0) {
                                *temp_v0_35 = 0;
                            }
                        }
                        M2C_FIELD(var_s3, s16 *, 4) = (s16) (M2C_FIELD(var_s3, s16 *, 4) - var_a0);
                        if (var_s1 == -1) {
                            temp_v1_8 = M2C_FIELD(var_s3, s16 *, 4);
                            if ((temp_v1_8 <= 0) && ((temp_v1_8 + var_a0) >= 0)) {
                                M2C_FIELD(var_s3, s16 *, 4) = 0;
                            }
                        }
                        temp_v1_9 = M2C_FIELD(var_s3, s16 *, 4);
                        if (temp_v1_9 < -0x1000) {
                            M2C_FIELD(var_s3, s16 *, 4) = (s16) (-0x2000 - temp_v1_9);
                            *temp_a1 = 0;
                        }
                    }
                    var_s4_5 += 1;
                } while (var_s4_5 != arg0);
            }
            temp_v0_13 = sp278 + 1;
            sp278 = temp_v0_13;
            var_s3 += 0x2C;
        } while ((s32) temp_v0_13 < (s8) *NULL);
        sp278 = NULL;
    }
    overlay47ExternalReloc((M2C_UNK *)0xFF, (M2C_UNK *)0xFF, (M2C_UNK *)0xFF, (M2C_UNK *)0xFF, (void *)0xFF);
    overlay47ExternalReloc((M2C_UNK *)2);
    var_s5 = 0;
    var_fp_3 = NULL;
    do {
        if (M2C_FIELD(var_fp_3, s8 *, 0x2B) != 0) {
            var_s1_4 = 0;
            if (M2C_FIELD(var_fp_3, s32 *, 0x24) != 0) {
                temp_t7_5 = (s32) sp278 * 4;
                temp_s7 = temp_t7_5 + &D_3DC;
                if (*(s32 *)0x30A == 4) {
                    var_s6_2 = (s32) ((f32) M2C_FIELD(temp_t7_5, s32 *, 0x530) + M2C_FIELD(var_fp_3, f32 *, 0x18));
                } else {
                    var_s6_2 = (s32) (M2C_FIELD(var_fp_3, f32 *, 0x18) - 20.0f);
                }
                var_s4_6 = 0x74;
                do {
                    var_s0_2 = var_s6_2;
                    if (var_s5 < 4) {
                        temp_v0_36 = var_s1_4 * 4;
                        if (*(s32 *)0x30A != 4) {
                            overlay47ExternalReloc(NULL, var_s6_2 - 6, var_s4_6 + 2, M2C_FIELD((*NULL + temp_v0_36), M2C_UNK **, 0x244), (void *)9);
                        } else {
                            overlay47ExternalReloc(NULL, (M2C_UNK *)0xA0, var_s4_6 + 2, M2C_FIELD((*NULL + temp_v0_36), M2C_UNK **, 0x244), (void *)0xC);
                        }
                        var_s5 += 1;
                    }
                    temp_v1_10 = *NULL;
                    *NULL = temp_v1_10 + 8;
                    M2C_FIELD(temp_v1_10, M2C_UNK **, 4) = NULL;
                    M2C_FIELD(temp_v1_10, M2C_UNK **, 0) = (M2C_UNK *)0xE7000000;
                    temp_a2_2 = *NULL;
                    *NULL = temp_a2_2 + 8;
                    M2C_FIELD(temp_a2_2, M2C_UNK **, 0) = (M2C_UNK *)0xEF002C0F;
                    M2C_FIELD(temp_a2_2, M2C_UNK **, 4) = (M2C_UNK *)0x504340;
                    temp_a3_2 = *NULL;
                    *NULL = temp_a3_2 + 8;
                    M2C_FIELD(temp_a3_2, M2C_UNK **, 0) = (M2C_UNK *)0xB6000000;
                    M2C_FIELD(temp_a3_2, M2C_UNK **, 4) = (M2C_UNK *)0x10001;
                    temp_t0 = *NULL;
                    *NULL = temp_t0 + 8;
                    M2C_FIELD(temp_t0, M2C_UNK **, 4) = (M2C_UNK *)0xFFFDF6FB;
                    M2C_FIELD(temp_t0, M2C_UNK **, 0) = (M2C_UNK *)0xFCFFFFFF;
                    temp_v0_37 = *NULL;
                    temp_a0_5 = M2C_FIELD(((M2C_FIELD(M2C_FIELD(var_fp_3, s16 *, 0x28), s8 *, 0x524) * 4) + var_s1_4), s8 *, 0x4C8);
                    *NULL = temp_v0_37 + 8;
                    M2C_FIELD(temp_v0_37, M2C_UNK **, 0) = (M2C_UNK *)0xFA000000;
                    M2C_FIELD(temp_v0_37, M2C_UNK **, 4) = (M2C_UNK *) *temp_s7;
                    var_a0_2 = temp_a0_5 - 1;
                    if (temp_a0_5 != 0) {
                        do {
                            temp_v0_38 = *NULL;
                            *NULL = temp_v0_38 + 8;
                            temp_t6_4 = (((var_s0_2 + 6) & 0x3FF) << 0xE) | 0xF6000000 | (((var_s4_6 + 6) & 0x3FF) * 4);
                            temp_t9_5 = ((var_s0_2 & 0x3FF) << 0xE) | ((var_s4_6 & 0x3FF) * 4);
                            var_s0_2 += 8;
                            M2C_FIELD(temp_v0_38, M2C_UNK **, 4) = (M2C_UNK *) temp_t9_5;
                            M2C_FIELD(temp_v0_38, M2C_UNK **, 0) = (M2C_UNK *) temp_t6_4;
                            var_a0_2 -= 1;
                        } while (var_a0_2 != 0);
                    }
                    temp_v1_11 = *NULL;
                    *NULL = temp_v1_11 + 8;
                    M2C_FIELD(temp_v1_11, M2C_UNK **, 4) = NULL;
                    M2C_FIELD(temp_v1_11, M2C_UNK **, 0) = (M2C_UNK *)0xE7000000;
                    temp_v0_39 = *NULL;
                    *NULL = temp_v0_39 + 8;
                    M2C_FIELD(temp_v0_39, M2C_UNK **, 0) = (M2C_UNK *)0xFA000000;
                    M2C_FIELD(temp_v0_39, M2C_UNK **, 4) = (M2C_UNK *) (((s32) *temp_s7 & ~0xFF) | 0x40);
                    temp_t9_6 = M2C_FIELD(((M2C_FIELD(M2C_FIELD(var_fp_3, s16 *, 0x28), s8 *, 0x524) * 4) + var_s1_4), s8 *, 0x4C8);
                    var_s1_4 += 1;
                    temp_a0_6 = 5 - temp_t9_6;
                    var_a0_3 = temp_a0_6 - 1;
                    if (temp_a0_6 != 0) {
                        do {
                            temp_v0_40 = *NULL;
                            *NULL = temp_v0_40 + 8;
                            temp_t7_6 = (((var_s0_2 + 6) & 0x3FF) << 0xE) | 0xF6000000 | (((var_s4_6 + 6) & 0x3FF) * 4);
                            temp_t9_7 = ((var_s0_2 & 0x3FF) << 0xE) | ((var_s4_6 & 0x3FF) * 4);
                            var_s0_2 += 8;
                            M2C_FIELD(temp_v0_40, M2C_UNK **, 4) = (M2C_UNK *) temp_t9_7;
                            M2C_FIELD(temp_v0_40, M2C_UNK **, 0) = (M2C_UNK *) temp_t7_6;
                            var_a0_3 -= 1;
                        } while (var_a0_3 != 0);
                    }
                    var_s4_6 += 8;
                } while (var_s1_4 != (s32) (M2C_UNK *)4);
            }
        }
        var_fp_3 += 0x34;
        temp_t8_4 = sp278 + 1;
        sp278 = temp_t8_4;
    } while (temp_t8_4 != (M2C_UNK *)4);
    overlay47ExternalReloc(*(s32 *)0x314, &sp230, &sp22C, NULL);
    overlay47ExternalReloc(NULL, (M2C_UNK *)&D_4F0, (M2C_UNK *)(u32)((s32)sp230 - 0x20), (M2C_UNK *)(u32)((s32)sp22C - 6), (void *)(u32)1.0f, 1.0f, -2, 3);
    overlay47ExternalReloc(M2C_FIELD(&D_4F0, M2C_UNK **, 0), (M2C_UNK *) &D_548, (M2C_UNK *)2, (M2C_UNK *) &D_54C, (void *) arg0);
    M2C_FIELD(&D_4F0, s32 *, 8) = (s32) (D_54C * 65536.0f);
    if (sp250 != 0) {
        overlay47ExternalReloc((M2C_UNK *)0xC, NULL, NULL, (M2C_UNK *)0xA, (void *)1, 0.0f);
        *(s32 *)0x324 = 1;
    } else if (sp214 != 0) {
        overlay47ExternalReloc((M2C_UNK *)0xD, NULL, NULL, (M2C_UNK *)0xA);
        overlay47ExternalReloc((M2C_UNK *)0xC, NULL, NULL, (M2C_UNK *)4, (void *)1, 0.0f);
        *(s32 *)0x324 = 1;
    }
    overlay47ExternalReloc((M2C_UNK *)(u32)sp1F8, (M2C_UNK *)1);
    overlay47ExternalReloc(NULL, NULL);
    D_540 += D_544 * sp1FC;
    if (D_540 > 1.0f) {
        D_544 = -D_544;
        D_540 = 2.0f - D_540;
    } else if (D_540 < (f32)(u32)NULL) {
        D_540 = -D_540;
        D_544 = -D_544;
    }
    if (*(s32 *)0x30B != 0) {
        if (D_310 != NULL) {
            overlay47ExternalReloc(D_310);
        }
        overlay47ExternalReloc((M2C_UNK *)0xF, &D_310);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o047/func_overlay_047_F0000B30_1891948/func_overlay_047_F0000B30_1891948.s")
#endif
