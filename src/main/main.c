#include "ultra64.h"

/*
 * Resident main state and frame control, ROM 0x27760-0x2A250.
 *
 * PROVENANCE: the TU attribution and adopted function names are adapted from
 * Jet Force Gemini's published src/main.c and built src/main.c.o, a permitted
 * public retail-derived decomp under docs/CLEANROOM.md. Mickey's call graph
 * and six `main/main.c` string references establish the correspondence
 * independently (tiers B and C). JFG address-placeholder names are not used.
 */

extern s32 D_8007A164;
extern s32 D_8007A160;
extern s32 D_8007A170;
extern s32 D_8007A154;
extern s32 D_8007A14C;
extern s32 D_8007A148;
extern s32 D_8007A1B0;
extern s32 D_8007A1BC;
extern s32 D_8007A13C;
extern s32 D_8007A12C;
extern s32 D_8007A130;
extern s32 D_8007A134;
extern s32 D_8007A138;
extern s32 D_8007A1A4;
extern s32 D_8007A1A8;
extern s32 D_8007A1D4;
extern s32 D_8007A1EC;
extern u8 D_8007BEF4;
extern s8 D_800CF53F[];
extern void *D_800D18E0;
extern void *D_800D18E4;
extern u8 D_800D1928[];

#pragma GLOBAL_ASM("asm/nonmatchings/main/main/RevealReturnAddresses.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/main/mainThread.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/main/mainResetPressed.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/main/mainPreNMI.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/main/mainInitGame.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/main/func_80026FB4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/main/func_80027628.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/main/mainAddZBCheck.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/main/mainUpdateZBCheck.s")

/* PROVENANCE: body adapted from JFG src/main.c; Mickey byte identity is decisive. */
s8 mainGetZBCheck(s32 arg0) {
    if ((arg0 < 0) || (arg0 >= 8)) {
        return 1;
    }
    return D_800CF53F[arg0 * 8];
}

#pragma GLOBAL_ASM("asm/nonmatchings/main/main/mainCPUeffects.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/main/mainSetGameWindow.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/main/func_80027D14.s")

/* PROVENANCE: body adapted from JFG src/main.c; Mickey byte identity is decisive. */
s32 mainGameWindowChanging(void) {
    return D_8007A13C;
}

/* PROVENANCE: body adapted from JFG src/main.c; Mickey byte identity is decisive. */
void mainGameWindowSize(s32 *x1, s32 *y1, s32 *x2, s32 *y2) {
    *x1 = D_8007A12C;
    *y1 = D_8007A130;
    *x2 = D_8007A134;
    *y2 = D_8007A138;
}

#pragma GLOBAL_ASM("asm/nonmatchings/main/main/func_80027EC0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/main/func_80027FB8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/main/func_800282C8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/main/mainChangeLevel.s")

/* PROVENANCE: body adapted from JFG src/main.c; Mickey byte identity is decisive. */
void mainSetAnimGroup(s32 arg0) {
    D_8007A164 = arg0;
}

/* PROVENANCE: body adapted from JFG src/main.c; Mickey byte identity is decisive. */
s32 mainGetAnimGroup(void) {
    return D_8007A160;
}

/* PROVENANCE: body adapted from JFG src/main.c; Mickey byte identity is decisive. */
void mainChangeCameras(s32 arg0) {
    D_8007A170 = arg0;
}

/* PROVENANCE: body adapted from JFG src/main.c; Mickey byte identity is decisive. */
s32 mainGetNextCharacter(void) {
    return D_8007A154;
}

/* PROVENANCE: body adapted from JFG src/main.c; Mickey byte identity is decisive. */
s32 mainGetNextLevel(void) {
    return D_8007A14C;
}

#pragma GLOBAL_ASM("asm/nonmatchings/main/main/func_80028564.s")

/* PROVENANCE: body adapted from JFG src/main.c; Mickey byte identity is decisive. */
void mainSyncNextLevel(void) {
    D_8007A1B0 = 1;
}

s32 mainGetMode(void) {
    return D_8007A1BC;
}

/* PROVENANCE: body adapted from JFG src/main.c; Mickey byte identity is decisive. */
void mainSetMode(s32 modeToSet) {
    D_8007A1BC = modeToSet;
}

#pragma GLOBAL_ASM("asm/nonmatchings/main/main/func_80028D30.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/main/func_80028DE4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/main/func_80028E2C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/main/func_80028EA0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/main/func_80028EFC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/main/func_80028F3C.s")

void *func_80028F54(void) {
    return D_800D18E0;
}

#pragma GLOBAL_ASM("asm/nonmatchings/main/main/func_80028F60.s")

void func_80028F98(s32 arg0, s32 arg1, s32 arg2) {
}

void func_80028FA8(s32 arg0, s32 arg1, s32 arg2) {
}

s32 func_80028FB8(s32 arg0, s32 arg1, s32 arg2) {
    return 0;
}

#pragma GLOBAL_ASM("asm/nonmatchings/main/main/func_80028FCC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/main/func_80029038.s")

s32 func_8002904C(s32 arg0, s32 arg1) {
    return 0;
}

#pragma GLOBAL_ASM("asm/nonmatchings/main/main/func_8002905C.s")

void func_80029084(s32 arg0, s32 arg1) {
}

void func_80029090(s32 arg0, s32 arg1, s32 arg2) {
}

s32 func_800290A0(void) {
    return D_8007A1A8;
}

#pragma GLOBAL_ASM("asm/nonmatchings/main/main/func_800290AC.s")

s32 func_800290EC(void) {
    return D_8007A148;
}

void *func_800290F8(void) {
    return D_800D18E4;
}

#pragma GLOBAL_ASM("asm/nonmatchings/main/main/func_80029104.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/main/func_80029120.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/main/func_80029144.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/main/func_80029160.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/main/func_8002917C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/main/func_80029198.s")

void func_800291B4(void) {
    D_8007A1EC = 1;
}

u8 *func_800291C4(void) {
    return D_800D1928;
}

void func_800291D0(void) {
}

void func_800291D8(s32 arg0) {
    D_8007A1A4 = arg0;
}

s32 func_800291E4(void) {
    return D_8007A1D4;
}

/* PROVENANCE: body adapted from JFG src/main.c; Mickey byte identity is decisive. */
s32 mainGetNumberOfCameras(void) {
    return D_8007BEF4;
}

#pragma GLOBAL_ASM("asm/nonmatchings/main/main/func_800291FC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/main/func_80029240.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/main/func_80029274.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/main/func_800293D0.s")
