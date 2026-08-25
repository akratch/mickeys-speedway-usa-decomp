#include "PR/ultratypes.h"

typedef struct O58Unknown {
    s32 unk0;
    u8 unk4;
    u8 unk5;
    u8 unk6;
    u8 unk7;
    s32 unk8;
    u8 unkC[0xA];
    u8 unk16;
    u8 pad17[5];
    u8 unk1C;
    u8 unk1D;
    u8 unk1E;
    u8 pad1F[3];
    s16 unk22;
    s32 unk24;
    u8 pad28[8];
    s32 unk30;
    u8 pad34[0x24];
    s32 unk58;
    u8 pad5C[0xC];
    s32 unk68;
    u8 pad6C[0xC];
    s32 unk78;
    u8 pad7C[4];
    s32 unk80;
    u8 pad84[0x18];
    s32 unk9C;
    s32 unkA0;
    s32 unkA4;
    s32 unkA8;
    s32 unkAC;
    s32 unkB0;
    s32 unkB4;
    s32 unkB8;
    s32 unkBC;
    s32 unkC0;
    s32 unkC4;
    s32 unkC8;
    s32 unkCC;
    s32 unkD0;
    s32 unkD4;
    u8 padD8[0x6C];
    s32 unk144;
    u8 pad148[0x68];
    s32 unk1B0;
    u8 pad1B4[4];
    s32 unk1B8;
    s32 unk1BC;
    u8 pad1C0[0x7C];
    s32 unk23C;
    u8 pad240[6];
    s16 unk246;
    u8 pad248[0xA];
    s16 unk252;
    u8 pad254[0xA];
    s16 unk25E;
    u8 pad260[0xA];
    s16 unk26A;
    u8 pad26C[0xC];
    s32 unk278;
    u8 pad27C[0x50];
    s32 unk2CC;
    s32 unk2D0;
    s32 unk2D4;
    s32 unk1;
    s32 unk2;
    s32 unk3;
    s32 unk9;
    s32 unkA;
} O58Unknown;

#undef NULL
#define NULL 0

/*
 * Mickey-only mips_to_c scaffold. The nearest permitted donor skeleton is
 * JFG frontKeyboard at only 0.081 masked similarity. Normalized assembly
 * preserves the control flow but collapses most external relocations; the
 * overlay58ExternalReloc calls below therefore represent multiple unresolved
 * call targets and make no symbol-identity claim.
 *
 * NON_MATCHING plateau (2026-08-25): after repairing the complete 13-way
 * control-flow draft and sweeping all 119 flag combinations, -O2 -mips1 is
 * best at 69/3614 identical instruction words (3545 differ), 472 bytes short,
 * with the first mismatch at +0x0. The candidate uses a 0x2F0-byte frame while
 * retail uses 0x138. Correcting the update-rate ABI, known opening calls,
 * scalar menu state, table cursors, and indexed structure access did not
 * recover the target register lifetimes; the unresolved relocation types and
 * oversized compiler-managed stack state are the remaining blockers.
 */
#ifdef NON_MATCHING
extern O58Unknown *func_80028F54(void);
extern void func_80036AB0(O58Unknown *state, s32 updateRate);
extern void func_8004B0A4(s32 value);
O58Unknown *overlay58ExternalReloc(); /* provisional external relocation */
extern s32 D_108[];
extern O58Unknown *D_18;
extern O58Unknown *D_180;
extern u8 *D_23C;
extern O58Unknown *D_278;
extern s32 D_28;
extern s16 D_290;
extern s16 D_296;
extern s16 D_2A6;
extern s32 D_2B8;
extern s32 D_2C;
extern O58Unknown D_34;
extern s32 D_3C;
extern s32 D_40;
extern s32 D_44;
extern s32 D_48;
extern s32 D_4C;
extern s32 D_50;
extern s32 D_54;
extern s32 D_58;
extern s32 D_60;
extern s32 D_64;
extern s32 D_6C;
extern s32 D_70;
extern s32 D_74;
extern O58Unknown *D_78[];
extern O58Unknown *D_7C;
extern O58Unknown *D_80;
extern O58Unknown *D_84;
extern O58Unknown *D_88;
extern O58Unknown *D_8C;
extern u8 *D_90;
extern O58Unknown *D_94;
extern O58Unknown *D_98;
extern O58Unknown *D_A0;
extern O58Unknown *D_A4;
extern s32 D_A8[];
extern O58Unknown *D_B0;
extern O58Unknown *D_B4;
extern O58Unknown *D_BC;
extern O58Unknown *D_C0;
extern O58Unknown *D_C4;
extern O58Unknown *D_C8;
extern O58Unknown *D_D4;
extern s32 D_D8;
extern s32 D_DC;
extern O58Unknown D_E8;
extern O58Unknown *D_F8;

void func_overlay_058_F000138C_18B0574(s32 arg0) {
    s32 sp124;                                      /* compiler-managed */
    O58Unknown *sp10C;
    O58Unknown *sp108;
    O58Unknown *sp104;
    O58Unknown *sp100;
    s32 spF8;
    O58Unknown *spEC;
    O58Unknown *spE8;
    s32 spE4;
    s8 spD9;
    u8 spD8;
    O58Unknown *spC0;
    O58Unknown *spB8;
    s32 sp88;
    s16 sp86;
    s16 sp84;
    s32 sp80;
    s32 sp7C;
    O58Unknown *sp78;
    O58Unknown *sp64;
    s32 var_s0_11;
    O58Unknown *sp;
    s32 *var_s0_3;
    O58Unknown **var_s0_4;
    s32 var_s0_5;
    s32 var_s0_6;
    O58Unknown **var_s1;
    O58Unknown **var_s1_10;
    O58Unknown **var_s1_5;
    O58Unknown **var_s1_9;
    O58Unknown **var_s2;
    O58Unknown **var_s2_2;
    s32 *var_s3;
    s32 *var_s3_2;
    s32 *var_s3_3;
    O58Unknown **var_v0_2;
    s32 temp_a0;
    O58Unknown *temp_s0;
    s32 temp_s1;
    O58Unknown *temp_s1_2;
    O58Unknown *temp_s1_3;
    O58Unknown *temp_s4;
    O58Unknown *temp_s4_2;
    O58Unknown *temp_s4_3;
    O58Unknown *temp_t1;
    O58Unknown *temp_t3_2;
    O58Unknown *temp_t3_3;
    O58Unknown *temp_t4_4;
    O58Unknown *temp_t6;
    O58Unknown *temp_t6_2;
    O58Unknown *temp_t6_6;
    O58Unknown *temp_t7;
    O58Unknown *temp_t7_3;
    O58Unknown *temp_t7_6;
    O58Unknown *temp_t8_3;
    O58Unknown *temp_t8_4;
    O58Unknown *temp_t8_5;
    O58Unknown *temp_t8_6;
    s32 temp_t9;
    O58Unknown *temp_t9_6;
    O58Unknown *temp_v0_11;
    O58Unknown *temp_v0_12;
    O58Unknown *temp_v0_14;
    O58Unknown *temp_v0_15;
    O58Unknown *temp_v0_16;
    s32 temp_v0_17;
    O58Unknown *temp_v0_19;
    O58Unknown *temp_v0_21;
    O58Unknown *temp_v0_22;
    O58Unknown *temp_v0_23;
    O58Unknown *temp_v0_24;
    O58Unknown *temp_v0_3;
    O58Unknown *temp_v0_6;
    O58Unknown *temp_v0_7;
    O58Unknown *temp_v0_8;
    O58Unknown *var_a0;
    O58Unknown *var_s0_9;
    O58Unknown *var_s2_3;
    O58Unknown *var_s4_4;
    O58Unknown *var_s4_5;
    s32 var_s7_8;
    O58Unknown *var_v0;
    s16 *var_s0_14;
    s16 temp_s2;
    s16 temp_s5;
    s16 temp_s5_2;
    s16 temp_v1_3;
    s16 var_v0_4;
    s16 var_v1;
    s16 var_v1_3;
    s32 *var_s0_2;
    s32 temp_s0_2;
    s32 temp_s1_4;
    s32 temp_t1_2;
    s32 temp_t1_3;
    s32 temp_t2;
    s32 temp_t2_2;
    s32 temp_t2_3;
    s32 temp_t3;
    s32 temp_t4;
    s32 temp_t4_2;
    s32 temp_t4_3;
    s32 temp_t4_5;
    s32 temp_t4_6;
    s32 temp_t5;
    s32 temp_t5_2;
    s32 temp_t6_3;
    s32 temp_t6_4;
    s32 temp_t6_5;
    s32 temp_t7_2;
    s32 temp_t7_4;
    s32 temp_t7_5;
    s32 temp_t8;
    s32 temp_t8_2;
    s32 temp_t8_7;
    s32 temp_t9_2;
    s32 temp_t9_3;
    s32 temp_t9_4;
    s32 temp_t9_5;
    s32 temp_v0_2;
    s32 temp_v0_5;
    O58Unknown *temp_v1;
    O58Unknown *temp_v1_2;
    s32 var_a1;
    s32 var_a1_2;
    s32 var_a1_3;
    s32 var_s0_10;
    s32 var_s0_13;
    s32 var_s0_8;
    s32 var_s1_2;
    s32 var_s1_4;
    s32 var_s1_6;
    s32 var_s1_7;
    s32 var_s1_8;
    s32 var_s2_6;
    s32 var_s6;
    s32 var_s7;
    s32 var_s7_10;
    s32 var_s7_11;
    s32 var_s7_12;
    s32 var_s7_13;
    s32 var_s7_2;
    s32 var_s7_3;
    s32 var_s7_4;
    s32 var_s7_5;
    s32 var_s7_6;
    s32 var_s7_7;
    s32 var_s7_9;
    s32 var_v1_2;
    s32 var_v1_4;
    u8 **var_s2_4;
    u8 **var_s2_5;
    O58Unknown **var_s4;
    O58Unknown **var_s4_2;
    O58Unknown **var_s4_3;
    u8 *var_s0_12;
    u8 *var_v0_3;
    u8 temp_a2;
    u8 temp_v0;
    u8 temp_v0_10;
    u8 temp_v0_13;
    u8 temp_v0_4;
    u8 temp_v0_9;
    O58Unknown *temp_s2_2;
    void *temp_s2_3;
    s32 temp_t1_4;
    O58Unknown *temp_v0_18;
    u8 *temp_v0_20;
    u8 *var_s0;
    s32 var_s0_7;
    O58Unknown *var_s1_3;

    spB8 = func_80028F54();
    func_80036AB0(&D_E8, arg0);
    func_8004B0A4(0);
    temp_a0 = *(s32 *)0x44;
    if ((temp_a0 != 1) && (temp_a0 != 2)) {

    } else if ((*(s32 *)0) == -1) {
        temp_v0 = (u8) (*(s32 *)0);
        if ((s32) temp_v0 > 0) {
            var_s3 = &D_A8;
            var_s0 = NULL;
            do {
                temp_t9 = *var_s3;
                var_s3 += 4;
                var_s0 += 4;
                ((s32 *)var_s0)[-1] = D_108[(s32) temp_t9];
            } while ((u32) var_s3 < (u32) &(&D_A8)[temp_v0]);
        }
    }
    temp_v1 = (O58Unknown *)((s32) (*(s32 *)0) * 2);
    temp_s2 = temp_v1->unk252;
    spF8 = (s32) temp_v1->unk246;
    switch (temp_a0) {
    case 1:
        var_s7 = 0;
        overlay58ExternalReloc((O58Unknown *)0xFF, (O58Unknown **)0x80, NULL, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
        overlay58ExternalReloc(NULL, *(O58Unknown **)0x48 + *(s32 *)0x54 + 0xA0, (O58Unknown *)0x1E, (*(O58Unknown **)0)->unk9C, (O58Unknown *)4);
        overlay58ExternalReloc((O58Unknown *)0xFF, (O58Unknown **)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
        var_v0 = D_48;
        sp124 = (s32) (var_v0 + D_58);
        sp100 = (O58Unknown *) temp_s2;
        if ((s32) (u8) (*(s32 *)0) > 0) {
            var_s2 = &D_78;
            var_s3_2 = &D_A8;
            do {
                temp_v0_2 = -sp124;
                sp7C = 0;
                sp84 = temp_v0_2 + 0x4E;
                sp86 = (s16) (sp100 - 4);
                sp80 = 0;
                sp88 = 0;
                sp124 = temp_v0_2;
                sp78 = ((O58Unknown *)(((*var_s2)->unk0 * 4)))->unk144;
                overlay58ExternalReloc(NULL, &sp78, NULL, NULL, (O58Unknown *)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
                overlay58ExternalReloc((O58Unknown *)0xFF, (O58Unknown **)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
                overlay58ExternalReloc(NULL, sp124 + 0x28, sp100, ((O58Unknown *)((s32) *var_s3_2 * 4))->unk278, NULL);
                if (((*(s32 *)0) == 1) && ((spB8 + 4) == *var_s2)) {
                    overlay58ExternalReloc((O58Unknown *) D_E8.unk8, (O58Unknown **) D_E8.unk9, (O58Unknown *) D_E8.unkA, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
                } else {
                    overlay58ExternalReloc(NULL, (O58Unknown **)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
                }
                overlay58ExternalReloc(NULL, sp124 + 0x71, sp100, ((O58Unknown *)((*(s32 *)0) + ((*var_s2)->unk0 * 4)))->unk68, NULL);
                if (spB8->unk0 == 5) {
                    if (var_s7 == 0) {
                        overlay58ExternalReloc((*var_s2)->unk4, &sp108, &sp10C, &sp104);
                    } else {
                        overlay58ExternalReloc(D_78[0]->unk4 - (*var_s2)->unk4, &sp108, &sp10C, &sp104);
                    }
                } else if (var_s7 == 0) {
                    overlay58ExternalReloc((*var_s2)->unk4, &sp108, &sp10C, &sp104);
                } else {
                    overlay58ExternalReloc((*var_s2)->unk4 - D_78[0]->unk4, &sp108, &sp10C, &sp104);
                }
                overlay58ExternalReloc((O58Unknown *)0xFF, (O58Unknown **)0xFF, NULL, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
                temp_s4 = sp100 + *(s32 *)0x5C;
                if ((var_s7 == 0) || (sp108 != NULL)) {
                    overlay58ExternalReloc(&spC0, &D_3C, sp108);
                    overlay58ExternalReloc(NULL, sp124 + 0xDC, temp_s4, &spC0, (O58Unknown *)1);
                    overlay58ExternalReloc(NULL, sp124 + 0xDF, temp_s4, &D_40, NULL);
                }
                if (var_s7 != 0) {
                    if (spB8->unk0 == 5) {
                        overlay58ExternalReloc(NULL, sp124 + 0xBA, temp_s4, &D_44, NULL);
                    } else {
                        overlay58ExternalReloc(NULL, sp124 + 0xBA, temp_s4, &D_48, NULL);
                    }
                }
                overlay58ExternalReloc(&spC0, &D_4C, sp10C);
                overlay58ExternalReloc(NULL, sp124 + 0xE6, temp_s4, &spC0, NULL);
                overlay58ExternalReloc(NULL, sp124 + 0xFD, temp_s4, &D_54, NULL);
                overlay58ExternalReloc(&spC0, &D_58, sp104);
                overlay58ExternalReloc(NULL, sp124 + 0x104, temp_s4, &spC0, NULL);
                var_s7 += 1;
                var_s3_2 += 4;
                var_s2 += 4;
                sp100 += spF8;
            } while (var_s7 < (s32) (u8) (*(s32 *)0));
            var_v0 = *(s32 *)0x48;
        }
        temp_t9_2 = ((s32) arg0 * 0x10) - arg0;
        temp_t7 = var_v0 - temp_t9_2;
        D_48 = temp_t7;
        if ((s32) temp_t7 < 0) {
            temp_v0_3 = D_60;
            D_48 = NULL;
            if ((temp_v0_3 == NULL) && ((s32) (*(s32 *)0) & 0x9000)) {
                if ((*(s32 *)0x74 >= 4) && ((u8) (*(s32 *)0) == 0)) {
                    if (spB8->unk3 == 0) {
                        D_60 = (s32)(5);
                    } else {
                        D_60 = (s32)(6);
                    }
                } else {
                    (*(s32 *)0) = -1;
                    D_60 = (s32)(2);
                }
                overlay58ExternalReloc((O58Unknown *)0xC, NULL, &D_58);
            }
            if (temp_v0_3 != NULL) {
                D_54 += temp_t9_2;
                if ((temp_v0_3 == (O58Unknown *)5) || (temp_v0_3 == (O58Unknown *)6)) {
                    temp_t6 = D_58 + temp_t9_2;
                    D_58 = temp_t6;
                    if ((s32) temp_t6 >= 0x141) {
                        *(s32 *)0x44 = temp_v0_3;
                        overlay58ExternalReloc((O58Unknown *)0x1FA, NULL, &D_58);
                        return;
                    }
                } else {
                    temp_t9_3 = *(s32 *)0x5C + temp_t9_2;
                    *(s32 *)0x5C = temp_t9_3;
                    if ((temp_t9_3 >= 0xB5) && ((s32) D_54 >= 0x141)) {
                        *(s32 *)0x4C = 0x6E;
                        *(s32 *)0x44 = temp_v0_3;
                        overlay58ExternalReloc((O58Unknown *)0x1FA, NULL, &D_58);
                        D_60 = NULL;
                        D_64 = (s32)(0x4B);
                        *(s32 *)0x50 = 0;
                        return;
                    }
                }
            }
        } else {
        default:
            return;
        }
        break;
    case 2:
        var_s7_2 = 0;
        if ((s32) (*(s32 *)0) > 0) {
            temp_t8 = D_64 - arg0;
            D_64 = (s32)( temp_t8);
            if (temp_t8 < 0) {
                D_64 = (s32)(0xF);
                overlay58ExternalReloc((O58Unknown *)0x1B, NULL);
                temp_v0_4 = (u8) (*(s32 *)0);
                if ((s32) temp_v0_4 > 0) {
                    var_s0_2 = NULL;
                    do {
                        temp_v0_5 = *var_s0_2;
                        if (temp_v0_5 > 0) {
                            *var_s0_2 = temp_v0_5 - 1;
                        }
                        var_s0_2 += 4;
                    } while ((u32) var_s0_2 < (u32) (temp_v0_4 * 4));
                    var_s7_2 = 0;
                }
            }
        }
        overlay58ExternalReloc((O58Unknown *)0xFF, (O58Unknown **)0x80, NULL, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
        overlay58ExternalReloc(NULL, *(s32 *)0x48 + *(s32 *)0x50 + *(s32 *)0x54 + 0xA0, (O58Unknown *)0x1E, (*(O58Unknown **)0)->unkA0, (O58Unknown *)4);
        overlay58ExternalReloc((O58Unknown *)0xFF, (O58Unknown **)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
        spEC = D_48;
        spE4 = D_50;
        if ((s32) (u8) (*(s32 *)0) > 0) {
            var_s2_2 = &D_78;
            var_s3_3 = &D_A8;
            var_s0_3 = NULL;
            sp100 = (O58Unknown *) temp_s2;
            do {
                temp_t8_2 = -(s32) D_48;
                temp_t9_4 = -D_50;
                D_48 = (O58Unknown *) temp_t8_2;
                D_50 = temp_t9_4;
                sp78 = ((O58Unknown *)(((*var_s2_2)->unk0 * 4)))->unk144;
                sp7C = 0;
                sp84 = temp_t8_2 + temp_t9_4 + 0x4E;
                sp86 = (s16) (sp100 - 4);
                sp80 = 0;
                sp88 = 0;
                overlay58ExternalReloc(NULL, &sp78, NULL, NULL, (O58Unknown *)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
                overlay58ExternalReloc((O58Unknown *)0xFF, (O58Unknown **)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
                overlay58ExternalReloc(NULL, *(s32 *)0x48 + *(s32 *)0x50 + 0x28, sp100, ((O58Unknown *)((s32) *var_s3_3 * 4))->unk278, NULL);
                if (((*(s32 *)0) == 1) && ((spB8 + 4) == *var_s2_2)) {
                    overlay58ExternalReloc((O58Unknown *) *(u8 *)0xF0, (O58Unknown **) *(u8 *)0xF1, (O58Unknown *) *(u8 *)0xF2, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
                } else {
                    overlay58ExternalReloc(NULL, (O58Unknown **)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
                }
                overlay58ExternalReloc(NULL, *(s32 *)0x48 + *(s32 *)0x50 + 0x71, sp100, ((O58Unknown *)((*(s32 *)0) + ((*var_s2_2)->unk0 * 4)))->unk68, NULL);
                temp_s1 = (*var_s2_2)->unk22 - *var_s0_3;
                overlay58ExternalReloc((O58Unknown *)0xFF, (O58Unknown **)0xFF, NULL, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
                overlay58ExternalReloc(&spC0, &D_60, temp_s1);
                overlay58ExternalReloc(NULL, *(s32 *)0x48 + *(s32 *)0x4C + *(s32 *)0x50 + 0xBE, sp100, &spC0, NULL);
                overlay58ExternalReloc(&spC0, &D_64, *var_s0_3);
                overlay58ExternalReloc(NULL, *(s32 *)0x48 + *(s32 *)0x4C + *(s32 *)0x50 + 0xEB, sp100, &spC0, NULL);
                overlay58ExternalReloc(NULL, *(s32 *)0x48 + *(s32 *)0x4C + *(s32 *)0x50 + 0x113, sp100, &D_6C, NULL);
                var_s7_2 += 1;
                var_s0_3 += 4;
                var_s3_3 += 4;
                var_s2_2 += 4;
                sp100 += spF8;
            } while (var_s7_2 < (s32) (u8) (*(s32 *)0));
        }
        D_48 = spEC;
        D_50 = spE4;
        temp_t2 = ((s32) arg0 * 0x10) - arg0;
        *(s32 *)0x4C = (s32) (*(s32 *)0x4C - temp_t2);
        temp_t5 = D_50 - temp_t2;
        temp_t6_2 = D_54 - temp_t2;
        D_50 = temp_t5;
        D_54 = temp_t6_2;
        if ((*(s32 *)0x4C < 0) && (temp_t5 < 0) && ((s32) temp_t6_2 < 0)) {
            if (D_60 == NULL) {
                temp_v0_6 = (*(s32 *)0);
                if ((s32) temp_v0_6 & 0x9000) {
                    D_60 = (s32)(3);
                    overlay58ExternalReloc((O58Unknown *)0xC, NULL, &D_54);
                } else if (((s32) temp_v0_6 & 0x4000) && (spB8->unk0 != 5)) {
                    D_60 = (s32)(1);
                    overlay58ExternalReloc((O58Unknown *)0xD, NULL, &D_54);
                }
            } else {
                temp_t8_3 = D_48 + temp_t2;
                D_48 = temp_t8_3;
                if ((s32) temp_t8_3 >= 0x141) {
                    *(s32 *)0x5C = 0;
                    *(s32 *)0x44 = (O58Unknown *) D_60;
                    overlay58ExternalReloc((O58Unknown *)0x1FA, NULL, &D_54);
                    D_60 = NULL;
                    *(s32 *)0x54 = 0;
                }
            }
        }
        if (*(s32 *)0x4C < 0) {
            *(s32 *)0x4C = 0;
        }
        if (D_50 < 0) {
            D_50 = 0;
        }
        if ((s32) D_54 < 0) {
            D_54 = NULL;
            return;
        }
        break;
    case 3:
        var_s7_3 = 0;
        overlay58ExternalReloc((O58Unknown *)0xFF, (O58Unknown **)0x80, NULL, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
        overlay58ExternalReloc(NULL, *(s32 *)0x48 + *(s32 *)0x4C + 0xA0, (O58Unknown *)0x1E, (*(O58Unknown **)0)->unkA4, (O58Unknown *)4);
        overlay58ExternalReloc((O58Unknown *)0xFF, (O58Unknown **)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
        spEC = D_48;
        spE8 = D_4C;
        if ((s32) (u8) (*(s32 *)0) > 0) {
            sp100 = (O58Unknown *) temp_s2;
            var_s0_4 = &D_C0;
            var_s4 = &D_90;
            do {
                temp_t7_2 = -(s32) D_48;
                temp_t3 = -(s32) D_4C;
                D_48 = (O58Unknown *) temp_t7_2;
                D_4C = (O58Unknown *) temp_t3;
                sp78 = ((O58Unknown *)(((*var_s4)->unk0 * 4)))->unk144;
                sp7C = 0;
                sp84 = temp_t7_2 + temp_t3 + 0x4E;
                sp86 = (s16) (sp100 - 4);
                sp80 = 0;
                sp88 = 0;
                overlay58ExternalReloc(NULL, &sp78, NULL, NULL, (O58Unknown *)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
                overlay58ExternalReloc((O58Unknown *)0xFF, (O58Unknown **)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
                overlay58ExternalReloc(NULL, *(s32 *)0x48 + *(s32 *)0x4C + 0x28, sp100, ((O58Unknown *)((s32) *var_s0_4 * 4))->unk278, NULL);
                if (((*(s32 *)0) == 1) && ((spB8 + 4) == *var_s4)) {
                    overlay58ExternalReloc((O58Unknown *) *(s32 *)0xF0, (O58Unknown **) *(s32 *)0xF1, (O58Unknown *) *(s32 *)0xF2, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
                } else {
                    overlay58ExternalReloc(NULL, (O58Unknown **)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
                }
                overlay58ExternalReloc(NULL, *(s32 *)0x48 + *(s32 *)0x4C + 0x71, sp100, ((O58Unknown *)((*(s32 *)0) + ((*var_s4)->unk0 * 4)))->unk68, NULL);
                overlay58ExternalReloc(&spC0, &D_70, (O58Unknown *) (*var_s4)->unk22);
                overlay58ExternalReloc((O58Unknown *)0xFF, (O58Unknown **)0xFF, NULL, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
                overlay58ExternalReloc(NULL, *(s32 *)0x48 + *(s32 *)0x4C + 0xD2, sp100, &spC0, NULL);
                if ((*var_s4)->unk22 == 1) {
                    overlay58ExternalReloc(NULL, *(s32 *)0x48 + *(s32 *)0x4C + 0x104, sp100, &D_74, NULL);
                } else {
                    overlay58ExternalReloc(NULL, *(s32 *)0x48 + *(s32 *)0x4C + 0x104, sp100, &D_78, NULL);
                }
                var_s7_3 += 1;
                var_s4 += 4;
                var_s0_4 += 4;
                sp100 += spF8;
            } while (var_s7_3 < (s32) (u8) (*(s32 *)0));
        }
        temp_t6_3 = ((s32) arg0 * 0x10) - arg0;
        D_48 = spEC;
        temp_t1 = spEC - temp_t6_3;
        D_48 = temp_t1;
        D_4C = spE8;
        if ((s32) temp_t1 < 0) {
            D_48 = NULL;
            if (D_60 == NULL) {
                temp_v0_7 = (*(s32 *)0);
                if ((s32) temp_v0_7 & 0x9000) {
                    if (spB8->unk0 == 0) {
                        D_60 = (s32)(4);
                    } else {
                        D_60 = (s32)(0xC);
                    }
                    overlay58ExternalReloc((O58Unknown *)0xC, NULL, &D_4C);
                    return;
                }
                if ((s32) temp_v0_7 & 0x4000) {
                    D_60 = (s32)(2);
                    (*(s32 *)0) = -1;
                    overlay58ExternalReloc((O58Unknown *)0xD, NULL, &D_4C);
                    return;
                }
            } else {
                temp_t7_3 = D_4C + temp_t6_3;
                D_4C = temp_t7_3;
                if ((s32) temp_t7_3 >= 0x141) {
                    if (D_60 == 4) {
                        overlay58ExternalReloc((O58Unknown *) D_34.unk3);
                        D_34.unk0 = 1;
                        temp_v0_8 = overlay58ExternalReloc((O58Unknown *) D_34.unk3);
                        if (temp_v0_8 != NULL) {
                            overlay58ExternalReloc((O58Unknown *) D_34.unk3);
                            overlay58ExternalReloc((O58Unknown *) D_34.unk3);
                            temp_v0_8->unk16 = (u8) (temp_v0_8->unk16 | 2);
                        }
                        *(s32 *)0x30 = 2;
                        *(s32 *)0x38 = 0x50;
                        return;
                    }
                    *(s32 *)0x44 = (O58Unknown *) D_60;
                    overlay58ExternalReloc((O58Unknown *)0x1FA, NULL, &D_4C);
                    D_60 = NULL;
                    *(s32 *)0x4C = 0;
                    *(s32 *)0x50 = 0x140;
                    return;
                }
            }
        }
        break;
    case 12:
        var_s7_4 = 0;
        overlay58ExternalReloc((O58Unknown *)0xFF, (O58Unknown **)0x80, NULL, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
        overlay58ExternalReloc(NULL, *(s32 *)0x4C + *(s32 *)0x50 + 0xA0, (O58Unknown *)0x1E, (*(O58Unknown **)0)->unk1B8, (O58Unknown *)4);
        overlay58ExternalReloc((O58Unknown *)0xFF, (O58Unknown **)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
        temp_v0_9 = (u8) (*(s32 *)0);
        temp_v1_2 = (O58Unknown *)((s32) temp_v0_9 * 2);
        var_a1 = D_50;
        temp_s5 = temp_v1_2->unk26A;
        var_s0_5 = temp_v1_2->unk25E + *(s32 *)0x4C + var_a1;
        if ((s32) temp_v0_9 > 0) {
            var_s1 = &D_278;
            do {
                overlay58ExternalReloc(NULL, var_s0_5, (O58Unknown *)0x37, *var_s1, (O58Unknown *)4);
                var_s7_4 += 1;
                var_s1 += 4;
                var_s0_5 += temp_s5;
            } while (var_s7_4 < (s32) (u8) (*(s32 *)0));
            var_a1 = *(s32 *)0x50;
            var_s7_4 = 0;
        }
        spE8 = (O58Unknown *) *(s32 *)0x4C;
        spE4 = var_a1;
        sp100 = (O58Unknown *) temp_s2;
        if ((s32) (u8) (*(s32 *)0) > 0) {
            var_s4_2 = &D_90;
            do {
                temp_t2_2 = -(s32) D_4C;
                temp_t9_5 = -D_50;
                D_4C = (O58Unknown *) temp_t2_2;
                D_50 = temp_t9_5;
                sp78 = ((O58Unknown *)((s32) (*var_s4_2)->unk0 * 4))->unk144;
                sp7C = 0;
                sp84 = temp_t2_2 + temp_t9_5 + 0x28;
                sp86 = (s16) (sp100 + 0x12);
                sp80 = 0;
                sp88 = 0;
                var_s1_2 = 0;
                overlay58ExternalReloc(NULL, &sp78, NULL, NULL, (O58Unknown *)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
                overlay58ExternalReloc((O58Unknown *)0xFF, (O58Unknown **)0xFF, NULL, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
                temp_v0_10 = (u8) (*(s32 *)0);
                var_s0_6 = ((O58Unknown *)((s32) temp_v0_10 * 2))->unk25E + *(s32 *)0x4C + *(s32 *)0x50;
                if ((s32) temp_v0_10 > 0) {
                    do {
                        overlay58ExternalReloc(&spC0, &D_7C, (O58Unknown *) (*var_s4_2)[var_s1_2].unk1C);
                        overlay58ExternalReloc(NULL, var_s0_6, sp100 + 0x16, &spC0, (O58Unknown *)4);
                        var_s1_2 += 1;
                        var_s0_6 += temp_s5;
                    } while (var_s1_2 < (s32) (u8) (*(s32 *)0));
                }
                var_s7_4 += 1;
                var_s4_2 += 4;
                sp100 += spF8;
            } while (var_s7_4 < (s32) (u8) (*(s32 *)0));
        }
        temp_t4 = ((s32) arg0 * 0x10) - arg0;
        D_50 = spE4;
        temp_t6_4 = spE4 - temp_t4;
        D_50 = temp_t6_4;
        D_4C = spE8;
        if (temp_t6_4 < 0) {
            *(s32 *)0x50 = 0;
            if (D_60 == NULL) {
                temp_v0_11 = (*(s32 *)0);
                if ((s32) temp_v0_11 & 0x9000) {
                    if (spB8->unk0 == 3) {
                        D_60 = (s32)(4);
                    } else {
                        D_60 = (s32)(7);
                    }
                    overlay58ExternalReloc((O58Unknown *)0xC, NULL);
                    return;
                }
                if ((s32) temp_v0_11 & 0x4000) {
                    D_60 = (s32)(3);
                    (*(s32 *)0) = -1;
                    overlay58ExternalReloc((O58Unknown *)0xD, NULL);
                    return;
                }
            } else {
                temp_t8_4 = D_4C + temp_t4;
                D_4C = temp_t8_4;
                if ((s32) temp_t8_4 >= 0x141) {
                    if (D_60 == 4) {
                        overlay58ExternalReloc((O58Unknown *) D_34.unk3);
                        D_34.unk0 = 1;
                        temp_v0_12 = overlay58ExternalReloc((O58Unknown *) D_34.unk3);
                        if (temp_v0_12 != NULL) {
                            overlay58ExternalReloc((O58Unknown *) D_34.unk3);
                            overlay58ExternalReloc((O58Unknown *) D_34.unk3);
                            temp_v0_12->unk16 = (u8) (temp_v0_12->unk16 | 2);
                        }
                        *(s32 *)0x30 = 2;
                        *(s32 *)0x38 = 0x50;
                        return;
                    }
                    *(s32 *)0x44 = (O58Unknown *) D_60;
                    overlay58ExternalReloc((O58Unknown *)0x1FA, NULL);
                    D_60 = NULL;
                    *(s32 *)0x48 = (s32)(0x140);
                    *(s32 *)0x4C = 0;
                    *(s32 *)0x50 = (s32) (O58Unknown *)0x140;
                    return;
                }
            }
        }
        break;
    case 13:
        var_s7_5 = 0;
        overlay58ExternalReloc((O58Unknown *)0xFF, (O58Unknown **)0x80, NULL, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
        overlay58ExternalReloc(NULL, *(s32 *)0x4C + *(s32 *)0x50 + 0xA0, (O58Unknown *)0x1E, (*(O58Unknown **)0)->unk1BC, (O58Unknown *)4);
        overlay58ExternalReloc((O58Unknown *)0xFF, (O58Unknown **)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
        temp_v0_13 = (u8) (*(s32 *)0);
        if (temp_v0_13 == 4) {
            var_s6 = 4;
        } else {
            var_s6 = temp_v0_13 + 1;
        }
        var_a0 = D_4C;
        var_a1_2 = D_50;
        temp_s5_2 = ((O58Unknown *)((s32) var_s6 * 2))->unk26A;
        var_s0_7 = ((O58Unknown *)((s32) temp_v0_13 * 2))->unk25E + var_a0 + var_a1_2;
        if (var_s6 > 0) {
            var_s1_3 = NULL;
            do {
                sp7C = 0;
                sp84 = (s16) var_s0_7;
                sp86 = 0x37;
                sp80 = 0;
                sp88 = 0;
                sp78 = var_s1_3->unk144;
                overlay58ExternalReloc(NULL, &sp78, NULL, NULL, (O58Unknown *)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
                var_s7_5 += 1;
                var_s1_3 += 4;
                var_s0_7 += temp_s5_2;
            } while (var_s7_5 != var_s6);
            var_a0 = (O58Unknown *) *(s32 *)0x4C;
            var_a1_2 = *(s32 *)0x50;
            var_s7_5 = 0;
        }
        spE8 = var_a0;
        spE4 = var_a1_2;
        sp100 = (O58Unknown *) temp_s2;
        if ((s32) (u8) (*(s32 *)0) > 0) {
            var_s4_3 = &D_90;
            do {
                temp_t4_2 = -(s32) D_4C;
                temp_t6_5 = -D_50;
                D_4C = (O58Unknown *) temp_t4_2;
                D_50 = temp_t6_5;
                sp78 = ((O58Unknown *)((s32) (*var_s4_3)->unk0 * 4))->unk144;
                sp7C = 0;
                sp84 = temp_t4_2 + temp_t6_5 + 0x28;
                sp86 = (s16) (sp100 + 0x12);
                sp80 = 0;
                sp88 = 0;
                var_s1_4 = 0;
                overlay58ExternalReloc(NULL, &sp78, NULL, NULL, (O58Unknown *)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
                overlay58ExternalReloc((O58Unknown *)0xFF, (O58Unknown **)0xFF, NULL, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
                var_s0_8 = ((O58Unknown *)((u8) (*(s32 *)0) * 2))->unk25E + *(s32 *)0x4C + *(s32 *)0x50;
                if (var_s6 > 0) {
                    do {
                        overlay58ExternalReloc(&spC0, &D_80, (O58Unknown *) (*var_s4_3)[var_s1_4].unk24);
                        overlay58ExternalReloc(NULL, var_s0_8 + 8, sp100 + 0x16, &spC0, (O58Unknown *)4);
                        var_s1_4 += 1;
                        var_s0_8 += temp_s5_2;
                    } while (var_s1_4 != var_s6);
                }
                var_s7_5 += 1;
                var_s4_3 += 4;
                sp100 += spF8;
            } while (var_s7_5 < (s32) (u8) (*(s32 *)0));
        }
        temp_t1_2 = ((s32) arg0 * 0x10) - arg0;
        D_50 = spE4;
        temp_t7_4 = spE4 - temp_t1_2;
        D_50 = temp_t7_4;
        D_4C = spE8;
        if (temp_t7_4 < 0) {
            D_50 = 0;
            if (D_60 == NULL) {
                temp_v0_14 = (*(s32 *)0);
                if ((s32) temp_v0_14 & 0x9000) {
                    if (spB8->unk0 == 3) {
                        D_60 = (s32)(4);
                    } else {
                        D_60 = (s32)(7);
                    }
                    overlay58ExternalReloc((O58Unknown *)0xC, NULL, &D_4C);
                    return;
                }
                if ((s32) temp_v0_14 & 0x4000) {
                    D_60 = (s32)(0xC);
                    (*(s32 *)0) = -1;
                    overlay58ExternalReloc((O58Unknown *)0xD, NULL, &D_4C);
                    return;
                }
            } else {
                temp_t9_6 = D_4C + temp_t1_2;
                D_4C = temp_t9_6;
                if ((s32) temp_t9_6 >= 0x141) {
                    if (D_60 == 4) {
                        overlay58ExternalReloc((O58Unknown *) D_34.unk3);
                        D_34.unk0 = 1;
                        temp_v0_15 = overlay58ExternalReloc((O58Unknown *) D_34.unk3);
                        if (temp_v0_15 != NULL) {
                            overlay58ExternalReloc((O58Unknown *) D_34.unk3);
                            overlay58ExternalReloc((O58Unknown *) D_34.unk3);
                            temp_v0_15->unk16 = (u8) (temp_v0_15->unk16 | 2);
                        }
                        *(s32 *)0x30 = 2;
                        *(s32 *)0x38 = 0x50;
                        return;
                    }
                    *(s32 *)0x44 = (O58Unknown *) D_60;
                    overlay58ExternalReloc((O58Unknown *)0x1FA, NULL, &D_4C);
                    D_60 = NULL;
                    *(s32 *)0x4C = 0;
                    *(s32 *)0x50 = 0x140;
                    return;
                }
            }
        }
        break;
    case 7:
    case 11:
        var_s7_6 = 0;
        if (D_60 == NULL) {
            temp_v1_3 = (s16) (*(s32 *)0);
            if ((temp_v1_3 >= 0x11) && (D_D8 > 0)) {
                D_D8 -= 1;
                overlay58ExternalReloc((O58Unknown *)0xF, NULL);
            } else if ((temp_v1_3 < -0x10) && (D_D8 < 3)) {
                D_D8 += 1;
                overlay58ExternalReloc((O58Unknown *)0xF, NULL);
            }
        }
        sp124 = *(s32 *)0x4C + *(s32 *)0x50;
        overlay58ExternalReloc((O58Unknown *)0xFF, (O58Unknown **)0x80, NULL, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
        if (*(s32 *)0x44 == 7) {
            if ((*(s32 *)0) == 1) {
                overlay58ExternalReloc(NULL, sp124 + 0xA0, (O58Unknown *)0x1E, (*(O58Unknown **)0)->unk2D4, (O58Unknown *)4);
            } else {
                overlay58ExternalReloc(NULL, sp124 + 0xA0, (O58Unknown *)0x1E, (*(O58Unknown **)0)->unkA8, (O58Unknown *)4);
            }
        } else {
            overlay58ExternalReloc(NULL, sp124 + 0xA0, (O58Unknown *)0x1E, (*(O58Unknown **)0)->unkAC, (O58Unknown *)4);
        }
        var_s1_5 = &D_18;
        var_s0_9 = (O58Unknown *)0x50;
        do {
            if (var_s7_6 == D_D8) {
                overlay58ExternalReloc((O58Unknown *) D_E8.unk8, (O58Unknown **) D_E8.unk9, (O58Unknown *) D_E8.unkA, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
            } else {
                overlay58ExternalReloc((O58Unknown *)0xFF, (O58Unknown **)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
            }
            overlay58ExternalReloc(NULL, 0xA0 - sp124, var_s0_9, *var_s1_5, (O58Unknown *)4);
            var_s7_6 += 1;
            var_s0_9 += 0x1E;
            var_s1_5 += 4;
            sp124 = -sp124;
        } while (var_s7_6 < 4);
        temp_t4_3 = ((s32) arg0 * 0x10) - arg0;
        var_a1_3 = *(s32 *)0x50 - temp_t4_3;
        *(s32 *)0x50 = var_a1_3;
        if (var_a1_3 < 0) {
            if (D_60 == NULL) {
                temp_v0_16 = (*(s32 *)0);
                if ((s32) temp_v0_16 & 0x9000) {
                    D_60 = (s32)(4);
                    overlay58ExternalReloc((O58Unknown *)0xC, NULL);
                    var_a1_3 = *(s32 *)0x50;
                } else if ((s32) temp_v0_16 & 0x4000) {
                    if (*(s32 *)0x44 == 7) {
                        D_60 = (s32)(0xC);
                    } else {
                        D_60 = (s32)(0xA);
                    }
                    overlay58ExternalReloc((O58Unknown *)0xD, NULL);
                    var_a1_3 = *(s32 *)0x50;
                }
            } else {
                temp_t4_4 = D_4C + temp_t4_3;
                D_4C = temp_t4_4;
                if ((s32) temp_t4_4 >= 0x141) {
                    if (D_60 == 4) {
                        switch (D_D8) {             /* switch 1; irregular */
                        case 0:                     /* switch 1 */
                            overlay58ExternalReloc((*(s32 *)0), (O58Unknown **) var_a1_3);
                            overlay58ExternalReloc(&D_F8);
                            temp_v0_17 = (*(s32 *)0);
                            if (((temp_v0_17 == 2) || (temp_v0_17 == 3)) && ((u8) (*(s32 *)0) != 0)) {
                                (*(s32 *)0) = (s8) (4 - temp_v0_17);
                                (*(s32 *)0) = 4;
                            } else if ((temp_v0_17 == 1) && (spB8->unk0 == 5)) {
                                (*(s32 *)0) = 3;
                                (*(s32 *)0) = 4;
                            } else {
                                (*(s32 *)0) = 0;
                                (*(s32 *)0) = (s8) temp_v0_17;
                            }
                            spB8->unk8 = NULL;
                            spB8->unk30 = 0;
                            temp_v0_18 = spB8 + (2 * 0x28);
                            temp_v0_18->unk30 = 0;
                            temp_v0_18->unk58 = 0;
                            temp_v0_18->unk80 = 0;
                            temp_v0_18->unk8 = 0;
                            if (D_DC != 0) {
                                overlay58ExternalReloc((O58Unknown *) (s16) (*(s32 *)0), NULL, NULL, (O58Unknown *)5, (O58Unknown *)1, NULL);
                                D_DC = 0;
                            }
                            var_a1_3 = *(s32 *)0x50;
                            break;
                        case 1:                     /* switch 1 */
                            if (D_DC != 0) {
                                overlay58ExternalReloc((O58Unknown *)0x1D, NULL, NULL, (O58Unknown *)0xB, (O58Unknown *)1, NULL);
                                D_DC = 0;
                                var_a1_3 = *(s32 *)0x50;
                            }
                            break;
                        case 2:                     /* switch 1 */
                            if (D_DC != 0) {
                                if (spB8->unk0 == 5) {
                                    overlay58ExternalReloc((O58Unknown *)0xC, NULL, NULL, (O58Unknown *)0x12, (O58Unknown *)1, NULL);
                                } else {
                                    overlay58ExternalReloc((O58Unknown *)0xC, NULL, NULL, (O58Unknown *)0x11, (O58Unknown *)1, NULL);
                                }
                                D_DC = 0;
                                var_a1_3 = *(s32 *)0x50;
                            }
                            break;
                        case 3:                     /* switch 1 */
                            if (D_DC != 0) {
                                overlay58ExternalReloc((O58Unknown *)0xC, NULL, NULL, (O58Unknown *)0xC, (O58Unknown *)1, NULL);
                                D_DC = 0;
                                var_a1_3 = *(s32 *)0x50;
                            }
                            break;
                        }
                    } else {
                        *(s32 *)0x5C = 0;
                        *(s32 *)0x44 = (O58Unknown *) D_60;
                        overlay58ExternalReloc((O58Unknown *)0x1FA, NULL);
                        D_60 = NULL;
                        *(s32 *)0x48 = (s32)(0x140);
                        *(s32 *)0x4C = 0;
                        *(s32 *)0x50 = (s32) (O58Unknown *)0x140;
                        var_a1_3 = *(s32 *)0x50;
                        *(s32 *)0x58 = 0;
                    }
                }
            }
        }
        if (var_a1_3 < 0) {
            *(s32 *)0x50 = 0;
            return;
        }
        break;
    case 5:
        overlay58ExternalReloc((O58Unknown *)0xFF, (O58Unknown **)0xFF, NULL, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
        overlay58ExternalReloc(NULL, *(s32 *)0x58 + 0xA0, (O58Unknown *)0x50, (*(O58Unknown **)0)->unkB0, (O58Unknown *)4);
        *(s32 *)0x58 = (s32) -*(s32 *)0x58;
        overlay58ExternalReloc((O58Unknown *)0xFF, NULL, NULL, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
        overlay58ExternalReloc(NULL, *(s32 *)0x58 + 0xA0, (O58Unknown *)0x8C, (*(O58Unknown **)0)->unkB4, (O58Unknown *)4);
        temp_t2_3 = -*(s32 *)0x58;
        *(s32 *)0x58 = temp_t2_3;
        temp_t5_2 = temp_t2_3 - (((s32) arg0 * 0x10) - arg0);
        *(s32 *)0x58 = temp_t5_2;
        if (temp_t5_2 < 0) {
            if ((s32) (*(s32 *)0) & 0x9000) {
                overlay58ExternalReloc();
                if (((s32) (*(s32 *)0) > 0) && (spB8->unk0 == 0)) {
                    (*(s32 *)0) = -1;
                    if (D_DC != 0) {
                        overlay58ExternalReloc((O58Unknown *)0x12, NULL, NULL, (O58Unknown *)0xF, (O58Unknown *)1, NULL);
                        D_DC = 0;
                    }
                    overlay58ExternalReloc((O58Unknown *)1);
                } else if (D_DC != 0) {
                    overlay58ExternalReloc((O58Unknown *)0xC, NULL, NULL, (O58Unknown *)0xC, (O58Unknown *)1, NULL);
                    D_DC = 0;
                }
            }
            *(s32 *)0x58 = 0;
            return;
        }
        break;
    case 6:
        overlay58ExternalReloc((O58Unknown *)0xFF, (O58Unknown **)0xFF, NULL, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
        overlay58ExternalReloc(NULL, *(s32 *)0x58 + 0xA0, (O58Unknown *)0x50, (*(O58Unknown **)0)->unkB0, (O58Unknown *)4);
        D_58 = (O58Unknown *) -(s32) D_58;
        if (spB8->unk1 == 0) {
            overlay58ExternalReloc(NULL, (O58Unknown **)0xFF, NULL, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
            overlay58ExternalReloc(NULL, *(s32 *)0x58 + 0xA0, (O58Unknown *)0x8C, (*(O58Unknown **)0)->unk1B0, (O58Unknown *)4);
            D_58 = (O58Unknown *) -(s32) D_58;
        } else {
            var_s7_7 = 0;
            if ((u16) (*(s32 *)0) & 0x100) {
                overlay58ExternalReloc(&spC0, (*(O58Unknown **)0)->unk2CC);
                var_s0_10 = 0x6C;
            } else {
                temp_a2 = spB8->unk3;
                if (temp_a2 == 1) {
                    overlay58ExternalReloc(&spC0, (*(O58Unknown **)0)->unkB8, (O58Unknown *) temp_a2);
                    var_s0_10 = 0x94;
                } else {
                    overlay58ExternalReloc(&spC0, (*(O58Unknown **)0)->unkBC, (O58Unknown *) temp_a2);
                    var_s0_10 = 0x80;
                }
            }
            overlay58ExternalReloc(NULL, (O58Unknown **)0xFF, NULL, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
            overlay58ExternalReloc(NULL, *(s32 *)0x58 + 0xA0, (O58Unknown *)0x8C, &spC0, (O58Unknown *)4);
            temp_t1_3 = -(s32) D_58;
            D_58 = (O58Unknown *) temp_t1_3;
            var_v0_2 = &sp78;
            if ((s32) spB8->unk3 > 0) {
                var_v1 = var_s0_10 + temp_t1_3;
                do {
                    var_s7_7 += 1;
                    var_v0_2 += 0x10;
                    ((s32 *)var_v0_2)[-3] = 0;
                    ((s32 *)var_v0_2)[-1] = var_v1;
                    *((s16 *)var_v0_2 - 1) = 0xAA;
                    ((s32 *)var_v0_2)[-2] = 0;
                    ((s32 *)var_v0_2)[-4] = (s32) ((O58Unknown *)((s32) spB8->unk4 * 4))->unk144;
                    var_v1 += 0x28;
                } while (var_s7_7 < (s32) spB8->unk3);
            }
            (sp + (var_s7_7 * 0x10))->unk78 = 0;
            overlay58ExternalReloc(NULL, &sp78, NULL, NULL, (O58Unknown *)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
        }
        temp_t7_5 = D_58 - (((s32) arg0 * 0x10) - arg0);
        D_58 = temp_t7_5;
        if (temp_t7_5 < 0) {
            if ((s32) (*(s32 *)0) & 0x9000) {
                if (((s32) (*(s32 *)0) > 0) && (spB8->unk0 == 0)) {
                    (*(s32 *)0) = 6;
                    (*(s32 *)0) = 5;
                    overlay58ExternalReloc(&D_F8);
                    (*(s32 *)0) = (O58Unknown *) *(s16 *)((spB8->unk2 * 8) + (spB8->unk1 * 2));
                    (*(s32 *)0) = (O58Unknown *) spB8->unk4;
                    (*(s32 *)0) = 5;
                    (*(s32 *)0) = NULL;
                    overlay58ExternalReloc((O58Unknown *)0x12, NULL, NULL, (O58Unknown *)0xF, (O58Unknown *)1, NULL);
                    overlay58ExternalReloc((O58Unknown *)1);
                } else {
                    *(s32 *)0x70 = 1;
                }
            }
            D_58 = 0;
            return;
        }
        break;
    case 8:
        sp124 = *(s32 *)0x48 + *(s32 *)0x58;
        var_s7_8 = NULL;
        overlay58ExternalReloc((O58Unknown *)0xFF, (O58Unknown **)0x80, NULL, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
        overlay58ExternalReloc(NULL, sp124 + 0xA0, (O58Unknown *)0x1E, overlay58ExternalReloc((O58Unknown *) (s16) (*(s32 *)0)), (O58Unknown *)4);
        sp124 = -(s32) sp124;
        overlay58ExternalReloc((O58Unknown *)0xFF, (O58Unknown **)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
        overlay58ExternalReloc(NULL, sp124 + 0xA0, (O58Unknown *)0x39, (*(O58Unknown **)0)->unk9C, (O58Unknown *)4);
        var_v1_2 = D_2B8;
        if (var_v1_2 != 0) {
            if ((var_v1_2 >= 2) && (sp124 == 0)) {
                overlay58ExternalReloc((O58Unknown *)0x27C, NULL);
                var_v1_2 = 1;
                D_2B8 = 1;
            }
            if (var_v1_2 == 1) {
                overlay58ExternalReloc(NULL, &D_180, sp124 + 0x30, (O58Unknown *)0x24, (O58Unknown *)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
            }
        }
        var_s1_6 = 0;
        var_s2_3 = spB8;
        do {
            sp124 = -sp124;
            temp_s4_2 = *(s32 *)0x5C + var_s1_6 + 0x5B;
            if (var_s7_8 == *(s32 *)0x3C) {
                overlay58ExternalReloc((O58Unknown *) *(s32 *)0xF0, (O58Unknown **) *(s32 *)0xF1, (O58Unknown *) *(s32 *)0xF2, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
            } else {
                overlay58ExternalReloc((O58Unknown *)0xFF, (O58Unknown **)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
            }
            temp_s0 = var_s7_8 + 1;
            overlay58ExternalReloc(&spC0, (*(O58Unknown **)0)->unkC0, temp_s0);
            overlay58ExternalReloc(NULL, sp124 + 0x82, temp_s4_2, &spC0, (O58Unknown *)1);
            overlay58ExternalReloc(var_s2_3->unkC, &sp108, &sp10C, &sp104);
            if (var_s7_8 != *(s32 *)0x3C) {
                overlay58ExternalReloc((O58Unknown *)0xFF, (O58Unknown **)0xFF, NULL, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
            }
            overlay58ExternalReloc(&spC0, &D_84, sp108);
            overlay58ExternalReloc(NULL, sp124 + 0xB4, temp_s4_2, &spC0, (O58Unknown *)1);
            overlay58ExternalReloc(NULL, sp124 + 0xB7, temp_s4_2, &D_88, NULL);
            overlay58ExternalReloc(&spC0, &D_8C, sp10C);
            overlay58ExternalReloc(NULL, sp124 + 0xBE, temp_s4_2, &spC0, NULL);
            overlay58ExternalReloc(NULL, sp124 + 0xD5, temp_s4_2, &D_94, NULL);
            overlay58ExternalReloc(&spC0, &D_98, sp104);
            overlay58ExternalReloc(NULL, sp124 + 0xDC, temp_s4_2, &spC0, NULL);
            var_s7_8 = temp_s0;
            var_s1_6 += 0x1B;
            var_s2_3 += 4;
        } while (temp_s0 != (O58Unknown *)3);
        sp124 = -sp124;
        temp_s4_3 = temp_s4_2 + 0x2C;
        if (*(s32 *)0x40 != -1) {
            overlay58ExternalReloc((O58Unknown *) *(s32 *)0xF0, (O58Unknown **) *(s32 *)0xF1, (O58Unknown *) *(s32 *)0xF2, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
        } else {
            overlay58ExternalReloc((O58Unknown *)0xFF, (O58Unknown **)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
        }
        overlay58ExternalReloc(NULL, sp124 + 0x82, temp_s4_3, (*(O58Unknown **)0)->unkC4, (O58Unknown *)1);
        overlay58ExternalReloc(spB8->unk8, &sp108, &sp10C, &sp104);
        if (*(s32 *)0x40 == -1) {
            overlay58ExternalReloc((O58Unknown *)0xFF, (O58Unknown **)0xFF, NULL, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
        }
        overlay58ExternalReloc(&spC0, &D_A0, sp108);
        overlay58ExternalReloc(NULL, sp124 + 0xB4, temp_s4_3, &spC0, (O58Unknown *)1);
        overlay58ExternalReloc(NULL, sp124 + 0xB7, temp_s4_3, &D_A4, NULL);
        overlay58ExternalReloc(&spC0, &D_A8, sp10C);
        overlay58ExternalReloc(NULL, sp124 + 0xBE, temp_s4_3, &spC0, NULL);
        overlay58ExternalReloc(NULL, sp124 + 0xD5, temp_s4_3, &D_B0, NULL);
        overlay58ExternalReloc(&spC0, &D_B4, sp104);
        overlay58ExternalReloc(NULL, sp124 + 0xDC, temp_s4_3, &spC0, NULL);
        temp_t4_5 = ((s32) arg0 * 0x10) - arg0;
        temp_t8_5 = D_48 - temp_t4_5;
        D_48 = temp_t8_5;
        if ((s32) temp_t8_5 < 0) {
            temp_v0_19 = D_60;
            D_48 = NULL;
            if ((temp_v0_19 == NULL) && ((s32) (*(s32 *)0) & 0x9000)) {
                if ((*(s32 *)0x3C != -1) || (*(s32 *)0x40 != -1)) {
                    D_60 = (s32)(9);
                } else {
                    D_60 = (s32)(0xA);
                }
                overlay58ExternalReloc((O58Unknown *)0xC, NULL);
            }
            if (temp_v0_19 != NULL) {
                temp_t3_2 = D_58 + temp_t4_5;
                D_58 = temp_t3_2;
                if ((s32) temp_t3_2 >= 0x141) {
                    D_58 = NULL;
                    D_48 = 0x140;
                    *(s32 *)0x44 = temp_v0_19;
                    D_60 = NULL;
                    overlay58ExternalReloc((O58Unknown *)0x1FA, NULL, (O58Unknown *)0x140, &D_58);
                    return;
                }
            }
        }
        break;
    case 9:
        overlay58ExternalReloc((O58Unknown *)0xFF, (O58Unknown **)0x80, NULL, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
        temp_v0_20 = *(s32 *)0x48 + *(s32 *)0x58;
        sp124 = temp_v0_20;
        overlay58ExternalReloc(NULL, temp_v0_20 + 0xA0, (O58Unknown *)0x1E, (*(O58Unknown **)0)->unkC8, (O58Unknown *)4);
        overlay58ExternalReloc((O58Unknown *)0xFF, (O58Unknown **)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
        temp_t4_6 = ((s32) arg0 * 0x10) - arg0;
        temp_t8_6 = *(s32 *)0x48 - temp_t4_6;
        *(s32 *)0x48 = temp_t8_6;
        if ((s32) temp_t8_6 < 0) {
            *(s32 *)0x48 = NULL;
            if (D_60 == NULL) {
                temp_v0_21 = (*(s32 *)0);
                var_s7_9 = 0;
                if ((s32) temp_v0_21 & 0x4000) {
                    var_s7_9 = 1;
                } else if ((s32) temp_v0_21 & 0x9000) {
                    if ((D_28 == 2) && (D_2C == 9)) {
                        var_s7_9 = 1;
                    } else {
                        temp_v0_22 = (*(s32 *)0);
                        if ((temp_v0_22 == (O58Unknown *)3) || (D_28 == 3)) {
                            if (D_28 == 3) {
                                overlay58ExternalReloc((O58Unknown *)0xC, NULL);
                                D_60 = (s32)(0xA);
                                temp_s1_2 = overlay58ExternalReloc();
                                temp_s2_2 = temp_s1_2 + (((s32) overlay58ExternalReloc((O58Unknown *) (s16) (*(s32 *)0)) << 5));
                                if (*(s32 *)0x3C != -1) {
                                    temp_s2_2->unk1C = overlay58ExternalReloc((O58Unknown *) (u8) (*(s32 *)0));
                                    temp_s2_2->unk1D = overlay58ExternalReloc((O58Unknown *) *(u8 *)1);
                                    temp_s2_2->unk1E = overlay58ExternalReloc((O58Unknown *) *(u8 *)2);
                                }
                                if (*(s32 *)0x40 != -1) {
                                    (temp_s2_2 + (*(s32 *)0x40 * 8))->unk4 = overlay58ExternalReloc((O58Unknown *) (u8) (*(s32 *)0));
                                    (temp_s2_2 + (*(s32 *)0x40 * 8))->unk5 = overlay58ExternalReloc((O58Unknown *) *(s32 *)1);
                                    (temp_s2_2 + (*(s32 *)0x40 * 8))->unk6 = overlay58ExternalReloc((O58Unknown *) *(s32 *)2);
                                }
                                overlay58ExternalReloc(overlay58ExternalReloc((O58Unknown *) (s16) (*(s32 *)0)));
                                if (*(s32 *)0xE0 != 0) {
                                    overlay58ExternalReloc();
                                }
                            } else {
                                overlay58ExternalReloc((O58Unknown *)0xE, NULL);
                            }
                        } else {
                            temp_v0_22->unk0 = (u8) *(u8 *)(((O58Unknown *)((s32) D_28 * 4))->unk23C + D_2C);
                            if (temp_v0_22 == (O58Unknown *)2) {
                                D_28 = 3;
                            }
                            (*(s32 *)0) = (O58Unknown *) (temp_v0_22 + 1);
                            overlay58ExternalReloc((O58Unknown *)0xC, NULL);
                        }
                    }
                }
                if (var_s7_9 != 0) {
                    temp_v0_23 = (*(s32 *)0);
                    if (temp_v0_23 == NULL) {
                        overlay58ExternalReloc((O58Unknown *)0xE, NULL, NULL);
                    } else {
                        temp_v0_23->unk0 = 0x20U;
                        (*(s32 *)0) = temp_v0_23 - 1;
                        var_s2_4 = &D_23C;
                        var_s1_7 = 0;
                        do {
                            var_s7_10 = 0;
                            var_v0_3 = *var_s2_4;
loop_305:
                            if (*var_v0_3 == *((u8 *)temp_v0_23 - 1)) {
                                D_28 = var_s1_7;
                                D_2C = var_s7_10;
                            }
                            var_s7_10 += 1;
                            var_v0_3 += 1;
                            if (var_s7_10 != 0xA) {
                                goto loop_305;
                            }
                            var_s1_7 += 1;
                            var_s2_4 += 4;
                        } while (var_s1_7 != 3);
                        overlay58ExternalReloc((O58Unknown *)0xD, NULL, NULL);
                    }
                }
                var_v1_3 = (s16) (*(s32 *)0);
                if (var_v1_3 >= 0x11) {
                    if (D_28 == 0) {
                        overlay58ExternalReloc((O58Unknown *)0xE, NULL);
                    } else {
                        overlay58ExternalReloc((O58Unknown *)0xF, NULL);
                        D_28 -= 1;
                    }
                    var_v1_3 = (*(s32 *)0);
                }
                if (var_v1_3 < -0x10) {
                    if ((D_28 == 3) || (D_28 == 3)) {
                        overlay58ExternalReloc((O58Unknown *)0xE, NULL);
                    } else {
                        overlay58ExternalReloc((O58Unknown *)0xF, NULL);
                        D_28 += 1;
                    }
                }
                var_v0_4 = (s16) (*(s32 *)0);
                if (var_v0_4 < -0x10) {
                    if ((D_2C == 0) || (D_28 == 3)) {
                        overlay58ExternalReloc((O58Unknown *)0xE, NULL);
                    } else {
                        overlay58ExternalReloc((O58Unknown *)0xF, NULL);
                        D_2C -= 1;
                    }
                    var_v0_4 = (*(s32 *)0);
                }
                if (var_v0_4 >= 0x11) {
                    if ((D_2C == 9) || (D_28 == 3)) {
                        overlay58ExternalReloc((O58Unknown *)0xE, NULL);
                    } else {
                        overlay58ExternalReloc((O58Unknown *)0xF, NULL);
                        D_2C += 1;
                    }
                }
            }
            if (D_60 != NULL) {
                temp_t7_6 = D_58 + temp_t4_6;
                D_58 = temp_t7_6;
                if ((s32) temp_t7_6 >= 0x141) {
                    D_58 = NULL;
                    *(s32 *)0x48 = (s32)(0x140);
                    *(s32 *)0x44 = (O58Unknown *) D_60;
                    D_60 = NULL;
                    overlay58ExternalReloc((O58Unknown *)0x1FA, NULL, (O58Unknown *)0x140);
                }
            }
        }
        var_s4_4 = (O58Unknown *)0x78;
        var_s2_5 = &D_23C;
        var_s1_8 = 0;
        do {
            temp_t1_4 = sp124;
            var_s7_11 = 0;
            sp124 = -(s32) temp_t1_4;
            var_s0_11 = 0x34 - temp_t1_4;
loop_336:
            overlay58ExternalReloc(&spC0, &D_BC, (O58Unknown *) (*var_s2_5)[var_s7_11]);
            if ((var_s7_11 == D_2C) && (var_s1_8 == D_28)) {
                overlay58ExternalReloc((O58Unknown *) D_E8.unk8, (O58Unknown **) D_E8.unk9, (O58Unknown *) D_E8.unkA, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
            } else {
                overlay58ExternalReloc((O58Unknown *)0xFF, (O58Unknown **)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
            }
            overlay58ExternalReloc(NULL, var_s0_11, var_s4_4, &spC0, (O58Unknown *)4);
            var_s7_11 += 1;
            var_s0_11 += 0x18;
            if (var_s7_11 < 0xA) {
                goto loop_336;
            }
            var_s1_8 += 1;
            var_s2_5 += 4;
            var_s4_4 += 0x1B;
        } while (var_s1_8 < 3);
        sp124 = -sp124;
        if (D_28 == 3) {
            overlay58ExternalReloc((O58Unknown *) *(s32 *)0xF0, (O58Unknown **) *(s32 *)0xF1, (O58Unknown *) *(s32 *)0xF2, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
        } else {
            overlay58ExternalReloc((O58Unknown *)0xFF, (O58Unknown **)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
        }
        overlay58ExternalReloc(NULL, sp124 + 0xA0, var_s4_4, (*(O58Unknown **)0)->unkCC, (O58Unknown *)4);
        overlay58ExternalReloc((O58Unknown *)0xFF, (O58Unknown **)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
        var_s7_12 = 0;
        var_s1_9 = sp124 + 0x78;
        if ((s32) (*(s32 *)0) > 0) {
            var_s0_12 = NULL;
            do {
                overlay58ExternalReloc(&spC0, &D_C0, (O58Unknown *) *var_s0_12);
                overlay58ExternalReloc(NULL, var_s1_9, (O58Unknown *)0x50, &spC0, (O58Unknown *)4);
                var_s7_12 += 1;
                var_s0_12 += 1;
                var_s1_9 += 0x1E;
            } while (var_s7_12 < (s32) (*(s32 *)0));
        }
        if (((*(s32 *)0) != 3) && (D_28 != 3)) {
            overlay58ExternalReloc(&spC0, &D_C4, (O58Unknown *) *(u8 *)(((O58Unknown *)((s32) D_28 * 4))->unk23C + D_2C));
            overlay58ExternalReloc((O58Unknown *) *(s32 *)0xF0, (O58Unknown **) *(s32 *)0xF1, (O58Unknown *) *(s32 *)0xF2, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
            overlay58ExternalReloc(NULL, var_s1_9, (O58Unknown *)0x50, &spC0, (O58Unknown *)4);
            return;
        }
        break;
    case 10:
        sp124 = *(s32 *)0x48 + *(s32 *)0x58;
        var_s7_13 = 0;
        overlay58ExternalReloc((O58Unknown *)0xFF, (O58Unknown **)0x80, NULL, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
        overlay58ExternalReloc(NULL, sp124 + 0xA0, (O58Unknown *)0x1E, overlay58ExternalReloc((O58Unknown *) (s16) (*(s32 *)0)), (O58Unknown *)4);
        sp124 = -(s32) sp124;
        overlay58ExternalReloc((O58Unknown *)0xFF, (O58Unknown **)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
        overlay58ExternalReloc(NULL, sp124 + 0xA0, (O58Unknown *)0x39, (*(O58Unknown **)0)->unkD0, (O58Unknown *)4);
        overlay58ExternalReloc((O58Unknown *)0xFF, (O58Unknown **)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
        temp_s1_3 = overlay58ExternalReloc();
        var_v1_4 = D_2B8;
        temp_s2_3 = temp_s1_3 + (((s32) overlay58ExternalReloc((O58Unknown *) (s16) (*(s32 *)0)) << 5));
        if (var_v1_4 != 0) {
            if ((var_v1_4 >= 2) && (sp124 == 0)) {
                overlay58ExternalReloc((O58Unknown *)0x27C, NULL);
                var_v1_4 = 1;
                D_2B8 = 1;
            }
            if (var_v1_4 == 1) {
                overlay58ExternalReloc(NULL, &D_180, sp124 + 0x30, (O58Unknown *)0x24, (O58Unknown *)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
            }
        }
        var_s4_5 = (O58Unknown *)0x5B;
        sp64 = temp_s2_3;
        do {
            sp124 = -sp124;
            overlay58ExternalReloc(sp64->unk0, &sp108, &sp10C, &sp104);
            if (sp64->unk0 == NULL) {
                var_s0_13 = 0x4A;
                overlay58ExternalReloc(&spC0, &D_C8);
            } else {
                temp_s0_2 = ((s32) overlay58ExternalReloc((O58Unknown *) sp64->unk4, &D_C8)) & 0xFF;
                temp_s1_4 = ((s32) overlay58ExternalReloc((O58Unknown *) sp64->unk5)) & 0xFF;
                overlay58ExternalReloc(&spC0, &D_D4, (O58Unknown *) temp_s0_2, (O58Unknown *) temp_s1_4, overlay58ExternalReloc((O58Unknown *) sp64->unk6), sp108, sp10C, sp104);
                var_s0_13 = sp64->unk7 + 0x51;
            }
            var_s2_6 = var_s7_13 == *(s32 *)0x40;
            if (var_s2_6 == 0) {
                var_s2_6 = var_s7_13 == 3;
                if (var_s2_6 != 0) {
                    var_s2_6 = (*(s32 *)0x3C + 1) != 0;
                }
            }
            if (var_s2_6 != 0) {
                overlay58ExternalReloc((O58Unknown *) D_E8.unk8, (O58Unknown **) D_E8.unk9, (O58Unknown *) D_E8.unkA, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
            } else {
                overlay58ExternalReloc((O58Unknown *)0xFF, (O58Unknown **)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
            }
            if (var_s7_13 < 3) {
                overlay58ExternalReloc(NULL, sp124 + 0x28, var_s4_5, ((O58Unknown *)((s32) var_s7_13 * 4))->unk278, NULL);
            }
            sp7C = 0;
            sp84 = sp124 + 0x58;
            sp86 = (s16) (var_s4_5 - 4);
            sp80 = 0;
            sp88 = 0;
            sp78 = *(O58Unknown **)(var_s0_13 * 4);
            overlay58ExternalReloc(NULL, &sp78, NULL, NULL, (O58Unknown *)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
            var_s0_14 = &D_290;
            var_s1_10 = &spC0;
loop_370:
            if (var_s2_6 == 0) {
                if (var_s0_14 == &D_290) {
                    overlay58ExternalReloc(NULL, (O58Unknown **)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
                }
                if (var_s0_14 == &D_296) {
                    overlay58ExternalReloc((O58Unknown *)0xFF, (O58Unknown **)0xFF, NULL, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
                }
            }
            spD9 = 0;
            spD8 = *var_s1_10;
            overlay58ExternalReloc(NULL, *var_s0_14 + sp124, var_s4_5, (O58Unknown *) &spD8, NULL);
            var_s0_14 += 2;
            var_s1_10 += 1;
            if (var_s0_14 != &D_2A6) {
                goto loop_370;
            }
            if (var_s7_13 == 2) {
                var_s4_5 += 0x1B;
                overlay58ExternalReloc((O58Unknown *)0xFF, (O58Unknown **)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF, (O58Unknown *)0xFF);
                overlay58ExternalReloc(NULL, sp124 + 0xA0, var_s4_5, (*(O58Unknown **)0)->unkD4, (O58Unknown *)4);
            }
            var_s7_13 += 1;
            sp64 += 8;
            var_s4_5 += 0x1B;
        } while (var_s7_13 != 4);
        temp_t8_7 = ((s32) arg0 * 0x10) - arg0;
        temp_t6_6 = D_48 - temp_t8_7;
        D_48 = temp_t6_6;
        if ((s32) temp_t6_6 < 0) {
            temp_v0_24 = D_60;
            D_48 = NULL;
            if ((temp_v0_24 == NULL) && ((s32) (*(s32 *)0) & 0x9000)) {
                D_60 = (s32)(0xB);
                overlay58ExternalReloc((O58Unknown *)0xC, NULL);
            }
            if (temp_v0_24 != NULL) {
                temp_t3_3 = D_58 + temp_t8_7;
                D_58 = temp_t3_3;
                if ((s32) temp_t3_3 >= 0x141) {
                    *(s32 *)0x44 = temp_v0_24;
                    *(s32 *)0x4C = 0;
                    *(s32 *)0x50 = 0x140;
                    D_60 = NULL;
                    overlay58ExternalReloc((O58Unknown *)0x1FA, NULL, (O58Unknown *) 0x140);
                }
            }
        }
        break;
    }
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o058/func_overlay_058_F000138C_18B0574/func_overlay_058_F000138C_18B0574.s")
#endif
