#include "PR/ultratypes.h"
#include "tools/m2c/m2c_macros.h"

#undef NULL
#define NULL 0

extern s16 *D_410;
extern f32 D_390, D_394, D_398;
extern f32 D_2D0;
extern volatile f32 D_2D8;
extern f32 D_2EC, D_2FC;
extern volatile s16 D_2F0, D_2F2, D_2FA;
extern f32 G_rt_458c4;

extern void ext_o0_1ee14(void *, s8);
extern void ext_o0_1d4c0(void *, void *);
extern void ext_o0_29adc(s16 *, f32 *);
extern s32 ext_o0_1312c(f32, f32, void *, s32, s32);
extern f32 ext_o0_2a470(s16);
extern void ext_o0_5aac4(void *, void *, void *);
extern void ext_o0_19668(void *, void *, void *, void *);
extern void ext_o0_1d510(void *, void *, void *, void *, s32);
extern void ext_o0_2d98(void *);
extern void ext_o0_2b90(s32, f32, f32, f32, s32, void *);
extern void ext_o0_3e99c(void *, s32);

extern void func_overlay_009_F0000540_1866BB8(void *, void *, f32 *, s32);
extern void func_overlay_009_F0000744_1866DBC(void *, void *, f32 *, s32);
extern void func_overlay_009_F0000CE4_186735C(void *, void *, f32 *, f32);
extern void func_overlay_009_F0000F6C_18675E4(void *, f32 *, s32);
extern void func_overlay_009_F00010A4_186771C(void *, void *, f32);

#ifdef NON_MATCHING
void func_overlay_009_F0000000_1866678(void *object, s32 steps) {
    f32 vector[3];
    s16 angles[3];
    void *savedEntry;
    void *entry;
    f32 savedY;
    f32 stepFloat;
    f32 current;
    f32 target;
    f32 rate;
    f32 velocity;
    void *state;
    void *entryData;
    s32 remaining;
    s32 result;
    s8 timer;

    state = M2C_FIELD(object, void **, 0x64);
    D_410 = (s16 *)((u8 *)state + 0x1B8);
    M2C_FIELD(object, s32 *, 0x80) = 0;
    savedEntry = *M2C_FIELD(object, void ***, 0x68);
    ext_o0_1ee14(state, M2C_FIELD(state, s8 *, 0));
    G_rt_458c4 = D_390;

    if (M2C_FIELD(state, f32 *, 4) < -30.0f) {
        M2C_FIELD(state, f32 *, 4) = -30.0f;
    }
    if (M2C_FIELD(state, f32 *, 4) > 30.0f) {
        M2C_FIELD(state, f32 *, 4) = 30.0f;
    }
    if (M2C_FIELD(state, f32 *, 8) < -30.0f) {
        M2C_FIELD(state, f32 *, 8) = -30.0f;
    }
    if (M2C_FIELD(state, f32 *, 8) > 30.0f) {
        M2C_FIELD(state, f32 *, 8) = 30.0f;
    }

    ext_o0_1d4c0(object, state);
    angles[0] = -M2C_FIELD(state, s16 *, 0xF0);
    angles[1] = -M2C_FIELD(object, s16 *, 2);
    vector[2] = 0.0f;
    angles[2] = -M2C_FIELD(object, s16 *, 4);
    vector[0] = 0.0f;
    vector[1] = -1.0f;
    ext_o0_29adc(angles, vector);
    M2C_FIELD(state, f32 *, 0x60) = vector[0];
    M2C_FIELD(state, f32 *, 0x64) = vector[1];
    M2C_FIELD(state, f32 *, 0x5C) = vector[2];

    result = ext_o0_1312c(M2C_FIELD(object, f32 *, 0xC),
                           M2C_FIELD(object, f32 *, 0x14),
                           (u8 *)state + 0x68, 0x10000, 0);
    if ((result & 0x10000) &&
        ((M2C_FIELD(object, f32 *, 0x10) - 16.0f) <
         M2C_FIELD(state, f32 *, 0x68))) {
        M2C_FIELD(state, f32 *, 0x6C) =
            M2C_FIELD(state, f32 *, 0x68) -
            (M2C_FIELD(object, f32 *, 0x10) - 16.0f);
    } else {
        M2C_FIELD(state, f32 *, 0x6C) = 0.0f;
    }

    if ((M2C_FIELD(state, u8 *, 0x16C) == 0) ||
        (M2C_FIELD(state, u8 *, 0x16C) == 1)) {
        func_overlay_009_F0000744_1866DBC(object, state, &D_2D0, steps);
    }
    stepFloat = (f32)steps;
    func_overlay_009_F0000CE4_186735C(object, state, &D_2D0, stepFloat);
    func_overlay_009_F00010A4_186771C(object, state, stepFloat);
    func_overlay_009_F0000F6C_18675E4(object, &D_2D0, steps);
    func_overlay_009_F0000540_1866BB8(object, state, &D_2D0, steps);

    D_2F0 = (s16)(D_2F0 + (((s32)(3072.0f * D_2EC) + 0x400) * steps));
    *D_410++ = 0x22;
    *D_410++ = D_2F0;
    D_2F2 = (s16)(D_2F2 +
        (((s32)(3072.0f * (D_2D8 - 1.0f)) + 0x400) * steps));
    *D_410++ = 0x24;
    *D_410++ = D_2F2;
    D_2FA = (s16)(D_2FA + (steps << 8));
    *D_410++ = 0x2000;

    if ((M2C_FIELD((u8 *)M2C_FIELD(object, void **, 0x40) +
                       M2C_FIELD(object, u8 *, 0x93), s8 *, 0x1E) == 0) &&
        (savedEntry != NULL) && (M2C_FIELD(savedEntry, s16 *, 8) != 0)) {
        entryData = M2C_FIELD(savedEntry, void **, 0);
        savedY = M2C_FIELD(object, f32 *, 0x10);
        velocity = M2C_FIELD(state, f32 *, 4);
        if ((velocity < -2.0f) ||
            (M2C_FIELD(state, s32 *, 0x42C) < -0x14) ||
            (velocity > 2.0f) ||
            (M2C_FIELD(state, s32 *, 0x42C) >= 0x15)) {
            target = 0.0f;
            rate = D_394;
        } else {
            target = 4.0f;
            rate = D_398;
        }
        remaining = steps - 1;
        if (steps != 0) {
            current = D_2FC;
            do {
                current += (target - current) * rate;
            } while (remaining--);
            D_2FC = current;
        }
        M2C_FIELD(object, f32 *, 0x10) +=
            D_2FC * ext_o0_2a470(D_2FA);
        ext_o0_5aac4(savedEntry, entryData, object);
        ext_o0_19668(object, savedEntry, M2C_FIELD(object, void **, 0x50),
                      M2C_FIELD((u8 *)savedEntry +
                          (M2C_FIELD(savedEntry, s16 *, 0xA) * 4), void **, 0xC));
        M2C_FIELD(savedEntry, s16 *, 8) = 0;
        M2C_FIELD(object, f32 *, 0x10) = savedY;
    }

    M2C_FIELD(state, s8 *, 0x186) = 0;
    ext_o0_1d510(object, state, NULL, NULL, steps);
    if (M2C_FIELD(state, u8 *, 0x349) != 0) {
        if (M2C_FIELD(state, u8 *, 0x16C) == 1) {
            void *handle = M2C_FIELD(state, void **, 0xB8);
            M2C_FIELD(state, u8 *, 0x16C) = 0;
            M2C_FIELD(state, s8 *, 0x16E) = 8;
            if (handle != NULL) {
                ext_o0_2d98(handle);
            }
            ext_o0_2b90(6, M2C_FIELD(object, f32 *, 0xC),
                        M2C_FIELD(object, f32 *, 0x10),
                        M2C_FIELD(object, f32 *, 0x14), 4,
                        (u8 *)state + 0xB8);
        } else {
            timer = M2C_FIELD(state, s8 *, 0x16E);
            if (timer > 0) {
                M2C_FIELD(state, s8 *, 0x16E) = timer - steps;
            }
        }
    } else {
        M2C_FIELD(state, s8 *, 0x16E) = 0;
    }
    ext_o0_3e99c(object, steps);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o009/overlay9UpdateObjectState/func_overlay_009_F0000000_1866678.s")
#endif
