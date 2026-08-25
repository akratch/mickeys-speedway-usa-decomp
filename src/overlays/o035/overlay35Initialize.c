#include "ultra64.h"

typedef struct O35System {
    u8 pad0[0x82];
    u8 flag82;
    s8 mode83;
    u8 pad84[0xE];
    s16 handle92;
    u8 pad94[0x62];
    s8 flagF6;
} O35System;

extern void *D_o35_alloc;
extern s32 D_4;
extern O35System *D_o35_system;
extern s32 D_o35_reset1;
extern s32 D_o35_reset2;
extern void *D_o35_paramA;
extern void *D_o35_paramB;
extern s32 D_o35_reset3;
extern s32 D_o35_mask;
extern void *D_o35_handle;
extern s32 D_o35_zero;
extern u8 D_o35_byte;

extern void *call_o0_0_2AE30(s32, s32);
extern O35System *call_o0_0_2634C(void);
extern s32 call_o0_0_28DA0(void);
extern void call_o0_0_214D4(s32);
extern void call_o0_0_47D8(s32, s32);
extern void call_o0_0_509A0(s32);
extern void call_o0_0_4B90(s32);
extern void call_o0_0_15A40(void *, void *, s32, s32, s32);
extern void call_o0_0_15C48(s32, s32);
extern void call_o0_0_15BE4(void);
extern void call_o0_0_20114(s32);
extern void call_o0_0_C0F0(s32);
extern void *call_o0_0_33FF8(s16);
extern void call_o98_0_0(void *);
extern void func_overlay_035_F00001E0_1881EC0(s32);

/*
 * Plateau (2026-08-25): 106/120 masked words exact with the default flags;
 * first mismatch +0x30. Same-line grouping fixes the two-word tail cluster,
 * but the target's early volatile-slot address uses a2 where this candidate
 * uses t6, shifting temporary registers in three later clusters. The closest
 * JFG skeleton, trackInit, is assembly-only at the pinned reference commit.
 */
#ifdef NON_MATCHING
void func_overlay_035_F0000000_1881CE0(s32 arg0, s32 arg1, s32 arg2,
                                        s32 arg3, s32 arg4, s32 arg5) {
    O35System *system;

    D_o35_alloc = call_o0_0_2AE30(0x800, 0x8F);
    system = call_o0_0_2634C();
    *(O35System * volatile *)&D_o35_system = system;
    D_4 = 0;
    if ((system->flagF6 == 0) &&
        ((system->mode83 == 1) || (system->mode83 == 2))) {
        D_4 = 1;
    }

    func_overlay_035_F00001E0_1881EC0(arg0);
    D_o35_reset1 = 0;
    call_o0_0_214D4(call_o0_0_28DA0() - 1);
    call_o0_0_47D8(arg4, 0);
    call_o0_0_47D8(arg3, 1);
    call_o0_0_509A0(arg5);
    call_o0_0_4B90(arg2);

    D_o35_reset2 = 0;
    call_o0_0_15A40(D_o35_paramA, D_o35_paramB, 0x1F4, 0xC8, 0x64);
    call_o0_0_15C48(2, 0);
    call_o0_0_15C48(1, 0);
    call_o0_0_15BE4();
    call_o0_0_15C48(2, 0);
    call_o0_0_15C48(1, 0);
    call_o0_0_15BE4();

    system = D_o35_system;
    if (system->flag82 == 0) {
        call_o0_0_20114(0);
    }
    call_o0_0_C0F0(arg1);
    system = D_o35_system;
    if (system->flag82 == 0) {
        call_o0_0_20114(8);
        system = D_o35_system;
    }

    D_o35_reset3 = 0;
    D_o35_mask = 0x100000;
    if (system->handle92 != -1) {
        D_o35_handle = call_o0_0_33FF8(system->handle92);
        D_o35_zero = 0; system = D_o35_system; }
    D_o35_byte = 0;
    if (system->flagF6 != 0) {
        call_o98_0_0(D_o35_paramA);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o035/overlay35Initialize/func_overlay_035_F0000000_1881CE0.s")
#endif
