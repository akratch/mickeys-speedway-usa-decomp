#include "PR/ultratypes.h"
#include "tools/m2c/m2c_macros.h"
#undef NULL
#define NULL 0
#undef M2C_BITWISE
#define M2C_BITWISE(type, expr) ((type)(s32)(expr))

struct _m2c_stack_func_overlay_001_F000438C_185076C {
    /* 0x000 */ char pad0[0x4C];
    /* 0x04C */ s16 sp4C;                           /* inferred */
    /* 0x04E */ s16 sp4E;                           /* inferred */
    /* 0x050 */ M2C_UNK *sp50;                      /* inferred */
    /* 0x054 */ M2C_UNK *sp54;                      /* inferred */
    /* 0x058 */ s32 *sp58;                          /* inferred */
    /* 0x05C */ char pad5C[4];
    /* 0x060 */ s32 sp60;                           /* inferred */
    /* 0x064 */ char pad64[4];
    /* 0x068 */ f32 sp68;                           /* inferred */
    /* 0x06C */ char pad6C[0xE];                    /* maybe part of sp68[4]? */
    /* 0x07A */ s16 sp7A;                           /* inferred */
    /* 0x07C */ s16 sp7C;                           /* inferred */
    /* 0x07E */ s16 sp7E;                           /* inferred */
    /* 0x080 */ f32 sp80;                           /* inferred */
    /* 0x084 */ char pad84[4];
    /* 0x088 */ f32 sp88;                           /* inferred */
    /* 0x08C */ f32 sp8C;                           /* inferred */
    /* 0x090 */ f32 sp90;                           /* inferred */
    /* 0x094 */ char pad94[4];
    /* 0x098 */ f32 sp98;                           /* inferred */
    /* 0x09C */ char pad9C[4];
    /* 0x0A0 */ f32 spA0;                           /* inferred */
    /* 0x0A4 */ f32 spA4;                           /* inferred */
    /* 0x0A8 */ f32 spA8;                           /* inferred */
    /* 0x0AC */ f32 spAC;                           /* inferred */
    /* 0x0B0 */ char padB0[0x10];                   /* maybe part of spAC[5]? */
    /* 0x0C0 */ s32 spC0;                           /* inferred */
    /* 0x0C4 */ char padC4[0x10];                   /* maybe part of spC0[5]? */
    /* 0x0D4 */ f32 *spD4;                          /* inferred */
    /* 0x0D8 */ s16 *spD8;                          /* inferred */
    /* 0x0DC */ s16 spDC;                           /* inferred */
    /* 0x0DE */ s16 spDE;                           /* inferred */
    /* 0x0E0 */ s16 spE0;                           /* inferred */
    /* 0x0E2 */ char padE2[2];
    /* 0x0E4 */ f32 spE4;                           /* inferred */
    /* 0x0E8 */ f32 spE8;                           /* inferred */
    /* 0x0EC */ f32 spEC;                           /* inferred */
    /* 0x0F0 */ M2C_UNK spF0;                       /* inferred */
    /* 0x0F0 */ char padF0[0x40];
    /* 0x130 */ s32 *sp130;                         /* inferred */
    /* 0x134 */ char pad134[4];
};                                                  /* size = 0x138 */

s32 func_overlay_001_F00004B4_184C894();            /* extern */
M2C_UNK func_overlay_001_F0007D6C_185414C(s32, s32, s16, s16, s16, s16, s16 *, s16 *, void *); /* extern */
f32 *ext_o8_8();
void ext_o0_1ee0c();
void ext_o8_49dc();
void ext_o0_1d4c0();
void ext_o0_29adc();
s32 ext_o0_1312c(f32, f32, void *, u32, void *);
void ext_o0_1ecfc();
s32 ext_o2_123c(f32, f32, void *);
s16 ext_o0_2a4c0(f32, f32);
s16 ext_o0_2a5bc(s16, s16);
void ext_o7_edc();
f32 ext_o8_1000(void *, void *, f32);
s32 ext_o0_2630c();
void ext_o0_29598();
f32 ext_o0_2a46c(s16);
void ext_o0_2d98();
void ext_o0_2b90();
void ext_o0_2d70();
f32 ext_o0_2a428(f32, s32);
f32 ext_o0_2a470(s16);
s32 *ext_o0_1e174(void *, void *, f32);
s32 *ext_o0_1d920(void *, void *, f32);
s32 ext_o0_7cd8(void *, f32, f32, f32);
void ext_o8_49a4(f32, void *);
f32 ext_o8_34a0(void *, void *, f32, f32);
void ext_o8_49b4();
void ext_o0_1cfcc();
void ext_o8_3278();
void ext_o0_1d510();
void ext_o8_2ec0();
void ext_o8_3018();
void ext_o0_3e99c();
extern void *D_1D9C;
extern void *D_1DA0;
extern f32 D_4;
extern M2C_UNK D_8C;
extern M2C_UNK D_D4;
extern s32 G_o1_83e4;
extern f32 G_rt_458c4;
extern s32 G_rt_43a3c;
extern u8 G_offd_31a4;
extern u8 LOCAL_BSS[];
extern f32 LOCAL_DATA_4;
extern s32 LOCAL_BSS_1D78;
extern s32 LOCAL_BSS_1D94;
extern void *LOCAL_BSS_1BA4;
extern void *LOCAL_BSS_1D9C;
extern f32 LOCAL_RODATA_F8;
extern f32 LOCAL_RODATA_FC;
extern f32 LOCAL_RODATA_100;
extern f32 LOCAL_RODATA_104;
extern f32 LOCAL_RODATA_108;
extern f32 LOCAL_RODATA_10C;
extern f32 LOCAL_RODATA_110;
extern f32 LOCAL_RODATA_114;
extern f32 LOCAL_RODATA_118;
extern f32 LOCAL_RODATA_11C;
extern f32 LOCAL_RODATA_120;
extern f32 LOCAL_RODATA_124;
extern f32 LOCAL_RODATA_128;
extern f32 LOCAL_RODATA_12C;
extern f32 LOCAL_RODATA_130;
extern f32 LOCAL_RODATA_134;
extern f32 LOCAL_RODATA_138;
extern f32 LOCAL_RODATA_13C;
extern f32 LOCAL_RODATA_140;
extern f32 LOCAL_RODATA_144;
extern f32 LOCAL_RODATA_148;
extern f32 LOCAL_RODATA_14C;
extern f32 LOCAL_RODATA_150;
extern f32 LOCAL_RODATA_154;
extern f32 LOCAL_RODATA_158;
extern f32 LOCAL_RODATA_15C;

typedef struct HitRecord {
    f32 value;
    u32 flags;
} HitRecord;

void func_overlay_001_F000438C_185076C(f32 *arg0, s32 arg1) {
    f32 *sp130;
    HitRecord spF0[8];
    f32 spEC;
    f32 spE8;
    f32 spE4;
    s16 spE0;
    s16 spDE;
    s16 spDC;
    f32 spD8;
    f32 spD4;
    s32 spC0;
    f32 spAC;
    f32 spA8;
    f32 spA4;
    f32 spA0;
    f32 sp98;
    f32 sp90;
    f32 sp8C;
    f32 sp88;
    f32 sp80;
    s16 sp7E;
    s16 sp7C;
    s16 sp7A;
    f32 sp68;
    s32 sp60;
    s32 *sp58;
    M2C_UNK *sp54;
    M2C_UNK *sp50;
    s16 sp4E;
    s16 sp4C;
    M2C_UNK (*temp_a0_4)(M2C_UNK, M2C_UNK *);
    M2C_UNK (*temp_v0_15)(u8, M2C_UNK *);
    M2C_UNK *var_a0_2;
    M2C_UNK *var_a1;
    f32 temp_f0;
    f32 temp_f0_2;
    f32 temp_f0_3;
    f32 temp_f0_4;
    f32 temp_f0_5;
    f32 temp_f0_6;
    f32 temp_f0_7;
    f32 temp_f0_8;
    f32 temp_f10_2;
    f32 temp_f12;
    f32 temp_f12_3;
    f32 temp_f12_4;
    f32 temp_f12_5;
    f32 temp_f12_6;
    f32 temp_f12_7;
    f32 temp_f12_8;
    f32 temp_f14_2;
    f32 temp_f14_3;
    f32 temp_f16;
    f32 temp_f18;
    f32 temp_cos;
    f32 temp_f2;
    f32 temp_f2_2;
    f32 temp_f2_3;
    f32 temp_f2_4;
    f32 temp_f2_5;
    f32 temp_f2_6;
    f32 temp_f2_7;
    f32 temp_f2_8;
    f32 temp_f8_2;
    f32 var_f0;
    f32 var_f14;
    f32 var_f14_2;
    f32 var_f14_3;
    f32 var_f14_4;
    f32 var_f14_5;
    f32 var_f14_6;
    f32 var_f16;
    f32 var_f2;
    f32 var_f2_2;
    s16 *temp_a0;
    s16 *temp_a0_2;
    s16 *temp_a0_3;
    s16 *temp_a2;
    s16 *temp_s0;
    s16 temp_v0_14;
    s16 temp_v0_2;
    s16 temp_v1;
    s16 var_v0_2;
    s16 var_v0_3;
    s16 var_v0_4;
    s16 var_v0_5;
    s32 (*temp_v0_4)(M2C_UNK *);
    f32 *temp_v0;
    s32 *temp_v0_12;
    s32 temp_v0_3;
    s32 *var_v0_7;
    s32 temp_f10;
    s32 temp_f12_2;
    s32 temp_f14;
    s32 temp_f8;
    s32 temp_t6;
    s32 temp_v0_10;
    s32 temp_v0_13;
    s32 temp_v0_9;
    s32 temp_v1_2;
    s32 temp_v1_3;
    s32 var_a0;
    s32 var_v0_6;
    s32 var_v1_2;
    s32 var_v1_3;
    s32 var_v1_4;
    s32 var_v1_5;
    s8 temp_v0_8;
    u8 temp_a0_5;
    u8 temp_v0_11;
    u8 temp_v0_6;
    u8 var_v0;
    void *temp_v0_5;
    void *temp_v0_7;
    HitRecord *var_v1;

    temp_s0 = M2C_FIELD(arg0, s16 **, 0x64);
    if (func_overlay_001_F00004B4_184C894() != 0) {
        temp_v0 = ext_o8_8(temp_s0);
        sp130 = temp_v0;
        temp_t6 = G_o1_83e4;
        G_rt_458c4 = *temp_v0;
        if (temp_t6 == 1) {
            if ((G_rt_43a3c == 0) && (M2C_FIELD(temp_s0, s32 *, 0x438) == 0) && (M2C_FIELD(temp_s0, s16 *, 0x102) == 0) && !(M2C_FIELD(temp_s0, u16 *, 0x1A8) & 8)) {
                temp_v0_2 = M2C_FIELD(temp_s0, s16 *, 0x37C);
                if (temp_v0_2 != M2C_FIELD(temp_s0, s16 *, 0x3CC)) {
                    M2C_FIELD(temp_s0, s16 *, 0x3CC) = temp_v0_2;
                    M2C_FIELD(temp_s0, s16 *, 0x3CE) = 0;
                } else {
                    M2C_FIELD(temp_s0, s16 *, 0x3CE) = (s16) (M2C_FIELD(temp_s0, s16 *, 0x3CE) + arg1);
                }
                if ((f32) M2C_FIELD(temp_s0, s16 *, 0x3CE) > 360.0f) {
                    if (M2C_FIELD(temp_s0, u8 *, 0x170) == 0) {
                        M2C_FIELD(temp_s0, u8 *, 0x170) = 1U;
                    }
                    goto block_13;
                }
            } else {
block_13:
                M2C_FIELD(temp_s0, s16 *, 0x3CE) = 0;
            }
        }
        M2C_FIELD(temp_s0, f32 *, 0x70) = 0.0f;
        if ((M2C_FIELD(temp_s0, s8 *, 0x18D) != 0) || (M2C_FIELD(temp_s0, s16 *, 0x158) != 0) || (M2C_FIELD(temp_s0, u8 *, 0x170) != 0) || (M2C_FIELD(temp_s0, s16 *, 0x3FA) != 0)) {
            ext_o0_1ee0c(temp_s0, (f32 *)1);
        }
        LOCAL_BSS_1D94 = arg1;
        LOCAL_DATA_4 = (f32) arg1;
        sp68 = -M2C_FIELD(temp_s0, f32 *, 4);
        ext_o8_49dc(NULL);
        temp_f0 = LOCAL_RODATA_F8;
        M2C_FIELD(arg0, s32 *, 0x80) = 0;
        M2C_FIELD(temp_s0, s32 *, 0x428) = 0;
        M2C_FIELD(temp_s0, s32 *, 0x42C) = 0;
        M2C_FIELD(temp_s0, s32 *, 0x41C) = 0;
        M2C_FIELD(temp_s0, s32 *, 0x420) = 0;
        if (M2C_FIELD(temp_s0, f32 *, 4) < temp_f0) {
            M2C_FIELD(temp_s0, f32 *, 4) = temp_f0;
        }
        temp_f12 = LOCAL_RODATA_FC;
        if (temp_f12 < M2C_FIELD(temp_s0, f32 *, 4)) {
            M2C_FIELD(temp_s0, f32 *, 4) = temp_f12;
        }
        if (M2C_FIELD(temp_s0, f32 *, 8) < temp_f0) {
            M2C_FIELD(temp_s0, f32 *, 8) = temp_f0;
        }
        if (temp_f12 < M2C_FIELD(temp_s0, f32 *, 8)) {
            M2C_FIELD(temp_s0, f32 *, 8) = temp_f12;
        }
        ext_o0_1d4c0(arg0, temp_s0);
        spDC = -M2C_FIELD(temp_s0, s16 *, 0xF0);
        spDE = -M2C_FIELD(arg0, s16 *, 2);
        spEC = 0.0f;
        spE4 = 0.0f;
        spE0 = -M2C_FIELD(arg0, s16 *, 4);
        spE8 = -1.0f;
        ext_o0_29adc(&spDC, &spE4);
        M2C_FIELD(temp_s0, f32 *, 0x60) = spE4;
        M2C_FIELD(temp_s0, f32 *, 0x64) = spE8;
        M2C_FIELD(temp_s0, f32 *, 0x5C) = spEC;
        temp_v0_3 = ext_o0_1312c(M2C_FIELD(arg0, f32 *, 0xC), M2C_FIELD(arg0, f32 *, 0x14), NULL, 0x08010000, spF0);
        var_f0 = -32768.0f;
        var_a0 = temp_v0_3 - 1;
        M2C_FIELD(temp_s0, f32 *, 0x68) = -32768.0f;
        if (temp_v0_3 != NULL) {
            var_v1 = &spF0[var_a0];
            do {
                if (M2C_FIELD(var_v1, s32 *, 4) & 0x10000) {
                    M2C_FIELD(temp_s0, f32 *, 0x68) = (f32) M2C_FIELD(var_v1, f32 *, 0);
                }
                if (M2C_FIELD(var_v1, s32 *, 4) & 0x08000000) {
                    var_f0 = M2C_FIELD(var_v1, f32 *, 0);
                }
                var_v1 -= 1;
                var_a0 -= 1;
            } while (var_a0 != 0);
        }
        temp_f2 = M2C_FIELD(temp_s0, f32 *, 0x68);
        if (M2C_FIELD(arg0, f32 *, 0x10) < temp_f2) {
            M2C_FIELD(temp_s0, u8 *, 2) = 1U;
            M2C_FIELD(temp_s0, f32 *, 0x6C) = temp_f2;
        } else {
            M2C_FIELD(temp_s0, u8 *, 2) = 0U;
            M2C_FIELD(temp_s0, f32 *, 0x6C) = 0.0f;
        }
        if (M2C_FIELD(arg0, f32 *, 0x10) < var_f0) {
            if (M2C_FIELD(temp_s0, u8 *, 0x170) == 0) {
                M2C_FIELD(temp_s0, u8 *, 0x170) = 1U;
            }
            if ((M2C_FIELD(temp_s0, u8 *, 2) != 0) && (M2C_FIELD(temp_s0, u8 *, 3) != 1)) {
                ext_o0_1ecfc((s16 *) arg0, (f32 *) temp_s0);
            }
        }
        var_v0 = M2C_FIELD(temp_s0, u8 *, 0x192);
        if ((s32) var_v0 >= 0xB) {
            var_v0 = 0xA;
        }
        var_a0_2 = &D_8C;
        var_v1_2 = 1;
        spAC = (M2C_FIELD(sp130, f32 *, 0x40) + ((f32) var_v0 * M2C_FIELD(sp130, f32 *, 8))) * M2C_FIELD(temp_s0, f32 *, 0x3A0);
        do {
            temp_v0_4 = M2C_FIELD(var_a0_2, s32 (**)(M2C_UNK *), 0);
            if ((temp_v0_4 != NULL) && (M2C_FIELD(var_a0_2, u16 *, 0xC) & (1 << M2C_FIELD(temp_s0, u8 *, 0x381)))) {
                sp60 = var_v1_2;
                sp54 = var_a0_2;
                if (temp_v0_4(var_a0_2) != 0) {
                    M2C_FIELD(temp_s0, u8 *, 0x381) = (u8) var_v1_2;
                }
            }
            var_v1_2 += 1;
            var_a0_2 += 0x10;
        } while (var_v1_2 != 4);
        M2C_FIELD(LOCAL_BSS + (M2C_FIELD(temp_s0, u8 *, 0x381) * 0x10), M2C_UNK (**)(f32 *, f32 *), 0x80)(&spD8, &spD4);
        temp_a2 = (s16 *) LOCAL_BSS_1BA4;
        if (temp_a2 != NULL) {
            temp_v0_5 = LOCAL_BSS_1D9C;
            if (ext_o2_123c(M2C_FIELD(temp_v0_5, f32 *, 0xC), M2C_FIELD(temp_v0_5, f32 *, 0x14), temp_a2) != NULL) {
                temp_v0_5 = D_1D9C;
                M2C_FIELD(D_1DA0, f32 *, 0x3D0) = M2C_FIELD(temp_v0_5, f32 *, 0xC);
                M2C_FIELD(D_1DA0, f32 *, 0x3D4) = M2C_FIELD(temp_v0_5, f32 *, 0x14);
            }
            if (ext_o2_123c(spD8, spD4, LOCAL_BSS_1BA4) != NULL) {
                M2C_FIELD(D_1DA0, f32 *, 0x3D8) = spD8;
                M2C_FIELD(D_1DA0, f32 *, 0x3DC) = spD4;
            }
            temp_f14 = (s32) M2C_FIELD(D_1DA0, f32 *, 0x3D8);
            temp_f12_2 = (s32) M2C_FIELD(D_1DA0, f32 *, 0x3DC);
            func_overlay_001_F0007D6C_185414C(temp_f12_2, temp_f14, (s16) (s32) M2C_FIELD(D_1DA0, f32 *, 0x3D0), (s16) (s32) M2C_FIELD(D_1DA0, f32 *, 0x3D4), (s16) temp_f14, (s16) temp_f12_2, &sp4E, &sp4C, D_1D9C);
            spD8 = (f32) sp4E;
            spD4 = (f32) sp4C;
        }
        sp7C = ext_o0_2a4c0(spD8 - M2C_FIELD(arg0, f32 *, 0xC), spD4 - M2C_FIELD(arg0, f32 *, 0x14)) + 0x8000;
        sp7A = M2C_FIELD(LOCAL_BSS + (M2C_FIELD(temp_s0, u8 *, 0x381) * 0x10), s16 (**)(f32, f32), 0x84)(spD8, spD4);
        M2C_FIELD(temp_s0, f32 *, 0x3BC) = spD8;
        M2C_FIELD(temp_s0, f32 *, 0x3C0) = spD4;
        temp_f0_2 = (f32) ext_o0_2a5bc(M2C_FIELD(arg0, s16 *, 0), sp7C) * LOCAL_RODATA_100;
        var_f2 = temp_f0_2;
        M2C_FIELD(temp_s0, s32 *, 0x428) = (s32) (temp_f0_2 * LOCAL_RODATA_104);
        if (temp_f0_2 < 0.0f) {
            var_f2 = -temp_f0_2;
        }
        if (G_o1_83e4 == 1) {
            if (var_f2 > 24576.0f) {
                var_v0_2 = 0x6000;
            } else {
                var_v0_2 = (s16) (s32) var_f2;
            }
            temp_f8 = (s32) (var_f2 - 8192.0f);
            if (sp68 < (25.0f - ((f32) var_v0_2 * 0.0009765625f))) {
                M2C_FIELD(temp_s0, s32 *, 0x41C) = (s32) (M2C_FIELD(temp_s0, s32 *, 0x41C) | 0x8000);
            }
            var_v0_3 = (s16) temp_f8;
            if ((s16) temp_f8 < 0) {
                var_v0_3 = (s16) temp_f8 * -1;
            }
            if (((f32) var_v0_3 * LOCAL_RODATA_108) < sp68) {
                M2C_FIELD(temp_s0, s32 *, 0x41C) = (s32) (M2C_FIELD(temp_s0, s32 *, 0x41C) | 0x4000);
            }
            temp_f0_3 = (f32) ext_o0_2a5bc(sp7C, sp7A);
            var_f2_2 = temp_f0_3;
            if (temp_f0_3 < 0.0f) {
                var_f2_2 = -temp_f0_3;
            }
            if ((var_f2_2 * LOCAL_RODATA_10C) < sp68) {
                M2C_FIELD(temp_s0, s32 *, 0x41C) = (s32) (M2C_FIELD(temp_s0, s32 *, 0x41C) | 0x4000);
            }
        } else {
            if (var_f2 > 16384.0f) {
                var_v0_4 = 0x4000;
            } else {
                var_v0_4 = (s16) (s32) var_f2;
            }
            temp_f10 = (s32) (var_f2 - 16384.0f);
            if (sp68 < (25.0f - ((f32) var_v0_4 * 0.0014648438f))) {
                M2C_FIELD(temp_s0, s32 *, 0x41C) = (s32) (M2C_FIELD(temp_s0, s32 *, 0x41C) | 0x8000);
            }
            var_v0_5 = (s16) temp_f10;
            if ((s16) temp_f10 < 0) {
                var_v0_5 = (s16) temp_f10 * -1;
            }
            if (((f32) var_v0_5 * 0.006713867f) < sp68) {
                M2C_FIELD(temp_s0, s32 *, 0x41C) = (s32) (M2C_FIELD(temp_s0, s32 *, 0x41C) | 0x4000);
            }
            if (sp68 < LOCAL_RODATA_110) {
                M2C_FIELD(temp_s0, s32 *, 0x41C) = (s32) (M2C_FIELD(temp_s0, s32 *, 0x41C) | 0x8000);
            }
            temp_v0_6 = M2C_FIELD(temp_s0, u8 *, 0x171);
            if (temp_v0_6 != 0) {
                if (arg1 < (s32) temp_v0_6) {
                    M2C_FIELD(temp_s0, u8 *, 0x171) = (u8) (temp_v0_6 - arg1);
                } else {
                    M2C_FIELD(temp_s0, u8 *, 0x171) = 0U;
                }
                M2C_FIELD(temp_s0, s32 *, 0x42C) = -0x64;
                M2C_FIELD(temp_s0, s32 *, 0x41C) = 0x4000;
            }
        }
        if (G_rt_43a3c != 0) {
            M2C_FIELD(temp_s0, s32 *, 0x41C) = 0x4000;
            M2C_FIELD(arg0, f32 *, 0x1C) = 0.0f;
            M2C_FIELD(arg0, f32 *, 0x20) = 0.0f;
            M2C_FIELD(arg0, f32 *, 0x24) = 0.0f;
            M2C_FIELD(temp_s0, f32 *, 4) = 0.0f;
        }
        if ((LOCAL_BSS_1D78 != 0) && (G_rt_43a3c == 0)) {
            M2C_FIELD(temp_s0, s8 *, 0x183) = 1;
            M2C_FIELD(temp_s0, u8 *, 0x185) = 2U;
            M2C_FIELD(temp_s0, s8 *, 0x187) = 6;
            M2C_FIELD(temp_s0, f32 *, 0x188) = 1.0f;
        }
        if (G_rt_43a3c != 0) {
            ext_o7_edc();
        }
        if (M2C_FIELD(temp_s0, s32 *, 0x438) != 0) {
            M2C_FIELD(temp_s0, s32 *, 0x428) = 0;
            M2C_FIELD(temp_s0, s32 *, 0x42C) = 0;
            M2C_FIELD(temp_s0, s32 *, 0x41C) = 0;
            M2C_FIELD(temp_s0, s32 *, 0x420) = 0;
        }
        temp_v0_7 = M2C_FIELD(temp_s0, void **, 0xD4);
        if (temp_v0_7 != NULL) {
            spAC *= 1.0f + (LOCAL_RODATA_114 * M2C_FIELD(M2C_FIELD(temp_v0_7, void **, 0x64), f32 *, 0x14));
        }
        if (M2C_FIELD(temp_s0, u8 *, 0x185) == 0) {
            temp_f0_4 = M2C_FIELD(temp_s0, f32 *, 0x5C);
            if (temp_f0_4 != 0.0f) {
                var_f14 = 1.0f - (temp_f0_4 * 0.5f * M2C_FIELD(sp130, f32 *, 0xC));
                if (var_f14 < LOCAL_RODATA_118) {
                    var_f14 = LOCAL_RODATA_11C;
                }
                spAC *= var_f14;
            }
        }
        temp_v1 = M2C_FIELD(temp_s0, s16 *, 0x102);
        if ((temp_v1 != 0) && ((temp_v0_8 = M2C_FIELD(arg0, s8 *, 0x3B), (temp_v0_8 == 0x10)) || (temp_v0_8 == 0xF))) {
            var_f14_2 = M2C_FIELD(arg0, f32 *, 0x28) * 1.5f;
            if (var_f14_2 > 1.0f) {
                var_f14_2 = 1.0f;
            }
            if (temp_v1 > 0) {
                var_f14_2 = -var_f14_2;
            }
            M2C_FIELD(temp_s0, s16 *, 0x104) = (s16) (s32) (65536.0f * var_f14_2);
            if (M2C_FIELD(arg0, f32 *, 0x28) == 1.0f) {
                M2C_FIELD(temp_s0, s16 *, 0x102) = 0;
                M2C_FIELD(temp_s0, s16 *, 0x104) = 0;
            } else {
                M2C_FIELD(temp_s0, u8 *, 0x185) = 0U;
                M2C_FIELD(temp_s0, f32 *, 0x188) = 0.0f;
            }
        }
        temp_v1_2 = LOCAL_BSS_1D94;
        if (temp_v1_2 != 0) {
            spC0 = temp_v1_2 - 1;
            do {
                temp_f0_5 = ext_o8_1000(arg0, temp_s0, spAC);
                spAC = temp_f0_5;
                temp_v0_9 = M2C_FIELD(temp_s0, s32 *, 0x41C);
                temp_v1_3 = temp_v0_9 & 0x4000;
                if ((temp_v1_3 == 0) && (M2C_FIELD(temp_s0, f32 *, 0x5C) > 0.0f) && (M2C_FIELD(temp_s0, f32 *, 4) < -temp_f0_5)) {
                    var_v0_6 = 1;
                } else if ((temp_v1_3 == 0) && (M2C_FIELD(temp_s0, f32 *, 0x5C) < 0.0f)) {
                    var_v0_6 = 1;
                } else {
                    var_v0_6 = 0;
                    if (!(temp_v0_9 & 0xC000) && (M2C_FIELD(temp_s0, f32 *, 0x5C) > 0.0f)) {
                        var_v0_6 = 1;
                    }
                }
                if ((var_v0_6 != 0) && (M2C_FIELD(temp_s0, u8 *, 0x185) == 0)) {
                    M2C_FIELD(temp_s0, f32 *, 4) = (f32) (M2C_FIELD(temp_s0, f32 *, 4) + (G_rt_458c4 * M2C_FIELD(temp_s0, f32 *, 0x5C)));
                    temp_f2_2 = M2C_FIELD(sp130, f32 *, 0x1C);
                    if (temp_f2_2 < M2C_FIELD(temp_s0, f32 *, 4)) {
                        M2C_FIELD(temp_s0, f32 *, 4) = temp_f2_2;
                    }
                }
                if (M2C_FIELD(temp_s0, s16 *, 0x102) != 0) {
                    temp_f2_3 = LOCAL_RODATA_120;
                    M2C_FIELD(temp_s0, s32 *, 0x428) = 0;
                    M2C_FIELD(temp_s0, s32 *, 0x42C) = 0;
                    M2C_FIELD(temp_s0, s32 *, 0x41C) = 0;
                    M2C_FIELD(temp_s0, s32 *, 0x420) = 0;
                    M2C_FIELD(temp_s0, f32 *, 4) = (f32) (M2C_FIELD(temp_s0, f32 *, 4) * temp_f2_3);
                    M2C_FIELD(temp_s0, f32 *, 8) = (f32) (M2C_FIELD(temp_s0, f32 *, 8) * temp_f2_3);
                }
                temp_v0_10 = M2C_FIELD(temp_s0, s32 *, 0x41C);
                M2C_FIELD(temp_s0, s16 *, 0x100) = 0;
                if (temp_v0_10 & 0x4000) {
                    if (M2C_FIELD(temp_s0, f32 *, 4) < -5.0f) {
                        ext_o8_49dc((s16 *)1);
                    }
                    if (M2C_FIELD(temp_s0, f32 *, 4) < 0.0f) {
                        M2C_FIELD(temp_s0, f32 *, 4) = (f32) (M2C_FIELD(temp_s0, f32 *, 4) + M2C_FIELD(&sp130[(s32) -M2C_FIELD(temp_s0, f32 *, 4)], f32 *, 0xC8));
                        if ((M2C_FIELD(temp_s0, f32 *, 4) > 0.0f) && (M2C_FIELD(temp_s0, s32 *, 0x42C) >= -0x1E)) {
                            M2C_FIELD(temp_s0, f32 *, 4) = 0.0f;
                        }
                    } else if (M2C_FIELD(temp_s0, s32 *, 0x42C) < -0x1E) {
                        M2C_FIELD(temp_s0, f32 *, 4) = (f32) (M2C_FIELD(temp_s0, f32 *, 4) + M2C_FIELD(&sp130[(s32) M2C_FIELD(temp_s0, f32 *, 4)], f32 *, 0x20));
                        if (M2C_FIELD(temp_s0, f32 *, 4) > 6.0f) {
                            M2C_FIELD(temp_s0, f32 *, 4) = 6.0f;
                        }
                    } else {
                        M2C_FIELD(temp_s0, f32 *, 4) = (f32) (M2C_FIELD(temp_s0, f32 *, 4) - M2C_FIELD(sp130, f32 *, 0xC8));
                        if (M2C_FIELD(temp_s0, f32 *, 4) <= 0.0f) {
                            M2C_FIELD(temp_s0, f32 *, 4) = 0.0f;
                        }
                    }
                } else if (temp_v0_10 & 0x8000) {
                    temp_v0_11 = M2C_FIELD(temp_s0, u8 *, 0x185);
                    temp_f16 = -temp_f0_5;
                    if ((temp_v0_11 == 1) || (temp_v0_11 == 2)) {
                        if (G_offd_31a4 == 0) {
                            var_f14_3 = LOCAL_RODATA_124;
                        } else {
                            var_f14_3 = 0.5f;
                        }
                    } else {
                        temp_f2_4 = M2C_FIELD(temp_s0, f32 *, 4);
                        if (temp_f2_4 > 0.0f) {
                            var_v1_3 = (s32) temp_f2_4;
                            var_f14_4 = temp_f2_4 - (f32) var_v1_3;
                        } else {
                            temp_f12_3 = -temp_f2_4;
                            var_v1_3 = (s32) temp_f12_3;
                            var_f14_4 = temp_f12_3 - (f32) var_v1_3;
                        }
                        temp_v0_12 = &sp130[var_v1_3];
                        temp_f12_4 = M2C_FIELD(temp_v0_12, f32 *, 0x44);
                        var_f14_3 = ((M2C_FIELD(temp_v0_12, f32 *, 0x48) - temp_f12_4) * var_f14_4) + temp_f12_4;
                    }
                    if (M2C_FIELD(temp_s0, f32 *, 4) < temp_f16) {
                        M2C_FIELD(temp_s0, f32 *, 4) = (f32) (M2C_FIELD(temp_s0, f32 *, 4) * LOCAL_RODATA_128);
                        if (temp_f16 < M2C_FIELD(temp_s0, f32 *, 4)) {
                            goto block_160;
                        }
                    } else {
                        M2C_FIELD(temp_s0, f32 *, 4) = (f32) (M2C_FIELD(temp_s0, f32 *, 4) - (var_f14_3 * M2C_FIELD(temp_s0, f32 *, 0x3A0)));
                        if (M2C_FIELD(temp_s0, f32 *, 4) < temp_f16) {
block_160:
                            M2C_FIELD(temp_s0, f32 *, 4) = temp_f16;
                        }
                    }
                    if ((G_o1_83e4 == 1) && (G_offd_31a4 == 0) && (M2C_FIELD(temp_s0, u16 *, 0x1A8) & 1) && (ext_o0_2630c() == (s32)0x21) && (M2C_FIELD(temp_s0, s16 *, 0x37C) == 0x2A) && (LOCAL_RODATA_12C < M2C_FIELD(temp_s0, f32 *, 0x398))) {
                        M2C_FIELD(temp_s0, f32 *, 4) = (f32) (M2C_FIELD(temp_s0, f32 *, 4) - 2.0f);
                    }
                } else {
                    temp_f2_5 = M2C_FIELD(temp_s0, f32 *, 4);
                    if ((LOCAL_RODATA_130 < temp_f2_5) && (temp_f2_5 < LOCAL_RODATA_134)) {
                        M2C_FIELD(temp_s0, f32 *, 4) = 0.0f;
                    } else {
                        M2C_FIELD(temp_s0, f32 *, 4) = (f32) (temp_f2_5 * LOCAL_RODATA_138);
                    }
                }
                if ((LOCAL_RODATA_13C < M2C_FIELD(temp_s0, f32 *, 4)) && (M2C_FIELD(temp_s0, f32 *, 4) < LOCAL_RODATA_140)) {
                    ext_o0_29598(NULL, (f32 *)0x7F);
                }
                temp_v0_13 = M2C_FIELD(temp_s0, s32 *, 0x428);
                if (temp_v0_13 >= 0x42) {
                    var_v1_4 = -0x1F4;
                } else if (temp_v0_13 < -0x41) {
                    var_v1_4 = 0x1F4;
                } else {
                    var_v1_4 = (s32) (temp_v0_13 * -0x1F4) / 65;
                }
                temp_v0_14 = M2C_FIELD(temp_s0, s16 *, 0x108);
                var_f14_5 = -2.0f - M2C_FIELD(temp_s0, f32 *, 4);
                M2C_FIELD(temp_s0, s16 *, 0x108) = (s16) (temp_v0_14 + ((s32) (var_v1_4 - temp_v0_14) >> 1));
                if (var_f14_5 > 0.0f) {
                    var_f14_6 = (ext_o0_2a46c((s16) (s32) (var_f14_5 * LOCAL_RODATA_144)) * 0.25f) + 0.75f;
                } else {
                    if (var_f14_5 < 0.0f) {
                        var_f14_5 = -var_f14_5;
                    }
                    if (var_f14_5 > 2.0f) {
                        var_f14_5 = 2.0f;
                    }
                    var_f14_6 = (ext_o0_2a46c((s16) (s32) (var_f14_5 * 16384.0f)) + 1.0f) * 0.5f;
                }
                if (M2C_FIELD(temp_s0, f32 *, 4) > 0.0f) {
                    var_f14_6 = -var_f14_6;
                }
                M2C_FIELD(temp_s0, s16 *, 0xF0) = (s16) (s32) ((f32) M2C_FIELD(temp_s0, s16 *, 0xF0) + ((f32) M2C_FIELD(temp_s0, s16 *, 0x108) * var_f14_6));
                M2C_FIELD(temp_s0, f32 *, 8) = (f32) (M2C_FIELD(temp_s0, f32 *, 8) * LOCAL_RODATA_148);
                temp_f2_6 = M2C_FIELD(temp_s0, f32 *, 8);
                if ((LOCAL_RODATA_14C < temp_f2_6) && (temp_f2_6 < LOCAL_RODATA_150)) {
                    M2C_FIELD(temp_s0, f32 *, 8) = 0.0f;
                }
                spC0 -= 1;
            } while (spC0 != 0);
        }
        if (M2C_FIELD(temp_s0, s32 *, 0x420) & 0x2000) {
            temp_a0 = M2C_FIELD(temp_s0, s16 **, 0xA8);
            if (temp_a0 != NULL) {
                ext_o0_2d98(temp_a0);
            }
            ext_o0_2b90(4, M2C_FIELD(arg0, u32 *, 0xC), M2C_FIELD(arg0, u32 *, 0x10), M2C_FIELD(arg0, u32 *, 0x14), 4, temp_s0 + 0xA8);
        }
        if ((M2C_FIELD(temp_s0, s32 *, 0x41C) & 0x4000) && (M2C_FIELD(temp_s0, f32 *, 4) < 0.0f)) {
            temp_a0_2 = M2C_FIELD(temp_s0, s16 **, 0xAC);
            if (temp_a0_2 == NULL) {
                ext_o0_2b90(3, M2C_FIELD(arg0, u32 *, 0xC), M2C_FIELD(arg0, u32 *, 0x10), M2C_FIELD(arg0, u32 *, 0x14), 1, temp_s0 + 0xAC);
            } else {
                ext_o0_2d70(temp_a0_2, M2C_FIELD(arg0, u32 *, 0xC), M2C_FIELD(arg0, u32 *, 0x10), M2C_FIELD(arg0, u32 *, 0x14));
            }
        }
        temp_a0_3 = M2C_FIELD(temp_s0, s16 **, 0xAC);
        if (temp_a0_3 != NULL) {
            ext_o0_2d98(temp_a0_3);
        }
        M2C_FIELD(arg0, s16 *, 0) = (s16) (M2C_FIELD(temp_s0, s16 *, 0xF0) + M2C_FIELD(temp_s0, s16 *, 0x104));
        sp7E = M2C_FIELD(temp_s0, s16 *, 0xF0);
        if (M2C_FIELD(temp_s0, s32 *, 0x438) == 1) {
            temp_f0_6 = ext_o0_2a428(LOCAL_RODATA_154, LOCAL_BSS_1D94);
            temp_f2_7 = M2C_FIELD(temp_s0, f32 *, 4);
            if ((temp_f2_7 < -0.5f) || (temp_f2_7 > 0.5f)) {
                M2C_FIELD(temp_s0, f32 *, 4) = (f32) (temp_f2_7 * temp_f0_6);
            } else {
                M2C_FIELD(temp_s0, f32 *, 4) = 0.0f;
            }
            temp_f2_8 = M2C_FIELD(temp_s0, f32 *, 8);
            if ((temp_f2_8 < -0.5f) || (temp_f2_8 > 0.5f)) {
                M2C_FIELD(temp_s0, f32 *, 8) = (f32) (temp_f2_8 * temp_f0_6);
            } else {
                M2C_FIELD(temp_s0, f32 *, 8) = 0.0f;
            }
        }
        if (M2C_FIELD(temp_s0, u8 *, 0x181) != 0) {
            temp_f0_7 = (M2C_FIELD(temp_s0, f32 *, 0x84) * D_4) + (0.5f * M2C_FIELD(temp_s0, f32 *, 0x88) * D_4 * D_4);
            sp90 = M2C_FIELD(temp_s0, f32 *, 0x74) * temp_f0_7;
            sp8C = M2C_FIELD(temp_s0, f32 *, 0x78) * temp_f0_7;
            sp88 = M2C_FIELD(temp_s0, f32 *, 0x7C) * temp_f0_7;
            if (temp_f0_7 < 0.0f) {
                M2C_FIELD(temp_s0, f32 *, 0x84) = 0.0f;
                M2C_FIELD(temp_s0, f32 *, 0x88) = 0.0f;
                M2C_FIELD(temp_s0, u8 *, 0x181) = 0U;
                if (!(M2C_FIELD(temp_s0, s32 *, 0x41C) & 0x8000)) {
                    M2C_FIELD(temp_s0, f32 *, 4) = 0.0f;
                    M2C_FIELD(temp_s0, f32 *, 8) = 0.0f;
                }
            }
            temp_f0_8 = M2C_FIELD(temp_s0, f32 *, 0x84);
            sp80 = 1.0f - (temp_f0_8 / M2C_FIELD(temp_s0, f32 *, 0x80));
            M2C_FIELD(temp_s0, f32 *, 0x84) = (f32) (temp_f0_8 + (M2C_FIELD(temp_s0, f32 *, 0x88) * D_4));
            spA8 = ext_o0_2a470(sp7E) * M2C_FIELD(temp_s0, f32 *, 4) * sp80;
            var_f16 = ext_o0_2a46c(sp7E) * M2C_FIELD(temp_s0, f32 *, 4) * sp80;
        } else {
            sp90 = 0.0f;
            sp8C = 0.0f;
            sp88 = 0.0f;
            spA8 = ext_o0_2a470(sp7E) * M2C_FIELD(temp_s0, f32 *, 4);
            var_f16 = ext_o0_2a46c(sp7E) * M2C_FIELD(temp_s0, f32 *, 4);
        }
        spA4 = var_f16;
        spA8 += M2C_FIELD(temp_s0, f32 *, 8) * ext_o0_2a46c(sp7E);
        temp_cos = ext_o0_2a470(sp7E);
        temp_f14_2 = M2C_FIELD(arg0, f32 *, 0x20);
        temp_f12_5 = (spA8 * D_4) + sp90;
        temp_f10_2 = (var_f16 - (M2C_FIELD(temp_s0, f32 *, 8) * temp_cos)) * D_4;
        sp98 = ((temp_f14_2 * D_4) - (0.5f * G_rt_458c4 * D_4 * D_4)) + sp8C;
        temp_f8_2 = 1.0f / D_4;
        temp_f18 = temp_f10_2 + sp88;
        spA0 = temp_f8_2;
        M2C_FIELD(arg0, f32 *, 0x1C) = (f32) (temp_f12_5 * temp_f8_2);
        M2C_FIELD(arg0, f32 *, 0x20) = (f32) (temp_f14_2 - (G_rt_458c4 * D_4));
        M2C_FIELD(arg0, f32 *, 0xC) += temp_f12_5;
        M2C_FIELD(arg0, f32 *, 0x24) = (f32) (temp_f18 * spA0);
        M2C_FIELD(arg0, f32 *, 0x10) = (f32) (M2C_FIELD(arg0, f32 *, 0x10) + sp98);
        M2C_FIELD(arg0, f32 *, 0x14) += temp_f18;
        if (M2C_FIELD(temp_s0, s32 *, 0x2BC) == 1) {
            var_v0_7 = ext_o0_1e174(arg0, temp_s0, D_4);
        } else {
            var_v0_7 = ext_o0_1d920(arg0, temp_s0, D_4);
        }
        sp58 = var_v0_7;
        if ((ext_o0_7cd8(arg0, 0.0f, 0.0f, 0.0f) != NULL) || (M2C_FIELD(arg0, s16 *, 0x2E) == -1)) {
            if (M2C_FIELD(temp_s0, u8 *, 0x170) == 0) {
                M2C_FIELD(temp_s0, u8 *, 0x170) = 1U;
            }
            M2C_FIELD(arg0, f32 *, 0xC) = M2C_FIELD(temp_s0, f32 *, 0x38);
            M2C_FIELD(arg0, f32 *, 0x10) = (f32) M2C_FIELD(temp_s0, f32 *, 0x3C);
            M2C_FIELD(arg0, f32 *, 0x14) = M2C_FIELD(temp_s0, f32 *, 0x40);
            sp58 = var_v0_7;
            ext_o0_7cd8(arg0, 0.0f, 0.0f, 0.0f);
        }
        if (M2C_FIELD(temp_s0, s16 *, 0x166) != 0) {
            if (G_o1_83e4 == 3) {
                M2C_FIELD(temp_s0, u8 *, 0x171) = 0x78U;
                M2C_FIELD(temp_s0, s16 *, 0x166) = 0;
            } else if (M2C_FIELD(temp_s0, u8 *, 0x170) == 0) {
                M2C_FIELD(temp_s0, u8 *, 0x170) = 1U;
            }
        }
        M2C_FIELD(temp_s0, f32 *, 0x94) = (M2C_FIELD(arg0, f32 *, 0xC) - M2C_FIELD(temp_s0, f32 *, 0x38)) * spA0;
        M2C_FIELD(temp_s0, f32 *, 0x98) = (f32) ((M2C_FIELD(arg0, f32 *, 0x10) - M2C_FIELD(temp_s0, f32 *, 0x3C)) * spA0);
        M2C_FIELD(temp_s0, f32 *, 0x9C) = (M2C_FIELD(arg0, f32 *, 0x14) - M2C_FIELD(temp_s0, f32 *, 0x40)) * spA0;
        if ((M2C_FIELD(temp_s0, s16 *, 0x16A) == 0) && (var_v0_7 != NULL)) {
            temp_f12_6 = M2C_FIELD(temp_s0, f32 *, 0xC);
            M2C_FIELD(temp_s0, f32 *, 0xC) = (f32) (temp_f12_6 + ((3.0f - temp_f12_6) * (1.0f - ext_o0_2a428(LOCAL_RODATA_158, LOCAL_BSS_1D94))));
            temp_f12_7 = M2C_FIELD(temp_s0, f32 *, 0xC);
            temp_f14_3 = -temp_f12_7;
            if (M2C_FIELD(temp_s0, f32 *, 4) < temp_f14_3) {
                M2C_FIELD(temp_s0, f32 *, 4) = temp_f14_3;
            }
            if (temp_f12_7 < M2C_FIELD(temp_s0, f32 *, 4)) {
                M2C_FIELD(temp_s0, f32 *, 4) = temp_f12_7;
            }
            if (M2C_FIELD(temp_s0, f32 *, 8) < temp_f14_3) {
                M2C_FIELD(temp_s0, f32 *, 8) = temp_f14_3;
            }
            if (temp_f12_7 < M2C_FIELD(temp_s0, f32 *, 8)) {
                M2C_FIELD(temp_s0, f32 *, 8) = temp_f12_7;
            }
        } else {
            temp_f12_8 = M2C_FIELD(temp_s0, f32 *, 0xC);
            M2C_FIELD(temp_s0, f32 *, 0xC) = (f32) (temp_f12_8 + ((25.0f - temp_f12_8) * (1.0f - ext_o0_2a428(LOCAL_RODATA_15C, LOCAL_BSS_1D94))));
        }
        ext_o8_49a4(M2C_FIELD(temp_s0, f32 *, 0xC), temp_s0);
        M2C_FIELD(temp_s0, f32 *, 0x70) = ext_o8_34a0(arg0, temp_s0, spAC, LOCAL_DATA_4);
        ext_o8_49b4(temp_s0);
        ext_o0_1cfcc((s16 *) arg0, (f32 *) temp_s0, (s16 *) LOCAL_BSS_1D94);
        var_a1 = &D_D4;
        var_v1_5 = 2;
        do {
            if (var_v1_5 != M2C_FIELD(temp_s0, u8 *, 0x382)) {
                temp_a0_4 = M2C_FIELD(var_a1, M2C_UNK (**)(M2C_UNK, M2C_UNK *), 0);
                if ((temp_a0_4 != NULL) && (M2C_FIELD(var_a1, u16 *, 8) & (1 << M2C_FIELD(temp_s0, u8 *, 0x382)))) {
                    sp60 = var_v1_5;
                    sp50 = var_a1;
                    temp_a0_4(temp_a0_4, var_a1);
                }
            }
            var_v1_5 += 1;
            var_a1 += 0xC;
            temp_a0_5 = M2C_FIELD(temp_s0, u8 *, 0x382);
        } while (var_v1_5 != 6);
        temp_v0_15 = M2C_FIELD(LOCAL_BSS + (temp_a0_5 * 0xC), M2C_UNK (**)(u8, M2C_UNK *), 0xC0);
        if (temp_v0_15 != NULL) {
            temp_v0_15(temp_a0_5, var_a1);
        }
        ext_o8_3278((s16 *) arg0, (f32 *) temp_s0, (s16 *) LOCAL_BSS_1D94);
        ext_o0_1d510((s16 *) arg0, (f32 *) temp_s0, NULL, (f32 *)3, (M2C_UNK *) LOCAL_BSS_1D94);
        ext_o8_2ec0((s16 *) arg0, (f32 *) temp_s0, (s16 *) LOCAL_BSS_1D94);
        ext_o8_3018(arg0, temp_s0, M2C_FIELD(temp_s0, u32 *, 0x70), LOCAL_BSS_1D94);
        ext_o0_3e99c((s16 *) arg0, (f32 *) LOCAL_BSS_1D94);
        if ((M2C_FIELD(temp_s0, u8 *, 0x349) != 0) && (M2C_FIELD(temp_s0, u8 *, 0x16C) == 1)) {
            M2C_FIELD(temp_s0, u8 *, 0x16C) = 0U;
        }
    }
}
