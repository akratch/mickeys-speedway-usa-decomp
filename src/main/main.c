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
extern s32 D_8007A174;
extern s32 D_8007A178;
extern s32 D_8007A17C;
extern s32 D_8007A180;
extern s32 D_8007A184;
extern s32 D_8007A188;
extern s32 D_8007A18C;
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
extern u32 D_8007A1CC;
extern s32 D_8007A1D4;
extern s32 D_8007A1EC;
extern u8 D_8007BEF4;
extern u8 D_8007BEF8;
extern s8 D_800CF53F[];

typedef struct MainGameEntry {
    u8 pad0[4];
    u8 character;
    u8 pad5[35];
} MainGameEntry;

typedef struct MainCharacterState {
    u8 bytes[40];
} MainCharacterState;

typedef struct MainGameState {
    u8 flag0;
    u8 flag1;
    u8 pad2[2];
    MainCharacterState characters[6];
} MainGameState;

extern MainGameEntry *D_800D18E0;
extern void *D_800D18E4;
extern u8 D_800D1928[];
extern s32 levelNGetType(s32 level);
extern void func_80028EFC(MainCharacterState *, s32, s32);

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

void func_80028DE4(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5) {
    D_8007A174 = arg0;
    D_8007A178 = arg1;
    D_8007A17C = arg2;
    D_8007A180 = arg3;
    D_8007A184 = arg4;
    D_8007A188 = arg5;
    D_8007A18C = 1;
}

#pragma GLOBAL_ASM("asm/nonmatchings/main/main/mainFrontInit.s")

/* PROVENANCE: stripped function role follows JFG src/main.c order. */
void mainStartGame(void) {
}

void func_80028EA0(MainGameState *state) {
    s32 i;

    state->flag0 = 0;
    state->flag1 = 0;
    for (i = 0; i < 6; i++) {
        func_80028EFC(&state->characters[i], i, i);
    }
}

#pragma GLOBAL_ASM("asm/nonmatchings/main/main/func_80028EFC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/main/func_80028F3C.s")

void *func_80028F54(void) {
    return D_800D18E0;
}

u8 *func_80028F60(s32 index, s32 arg1) {
    if ((index < 0) || (index >= 6)) {
        index = 0;
    }
    return (u8 *) D_800D18E0 + (index * 40) + 4;
}

void func_80028F98(s32 arg0, s32 arg1, s32 arg2) {
}

void func_80028FA8(s32 arg0, s32 arg1, s32 arg2) {
}

s32 func_80028FB8(s32 arg0, s32 arg1, s32 arg2) {
    return 0;
}

#pragma GLOBAL_ASM("asm/nonmatchings/main/main/func_80028FCC.s")

s32 func_80029038(s32 arg0, s32 arg1, s32 arg2) {
    return 0;
}

s32 func_8002904C(s32 arg0, s32 arg1) {
    return 0;
}

s32 func_8002905C(s32 arg0) {
    return func_8002904C((s32) D_800D18E0, arg0);
}

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

void func_80029104(void) {
    D_8007A1CC |= 0x40000000;
}

void func_80029120(u32 arg0) {
    D_8007A1CC |= 0x20000000 | (arg0 & 0x1F);
}

void func_80029144(void) {
    D_8007A1CC |= 0x80000000;
}

void func_80029160(void) {
    D_8007A1CC |= 0x10000000;
}

void func_8002917C(void) {
    D_8007A1CC |= 0x08000000;
}

void func_80029198(void) {
    D_8007A1CC |= 0x00800000;
}

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

s32 func_800291FC(void) {
    s32 type = levelNGetType(D_8007A148);

    if ((type == 1) || (type == 2)) {
        return 0;
    }
    return D_8007BEF8;
}

u8 func_80029240(s32 index) {
    if ((index < 0) || (index >= 6)) {
        index = 0;
    }
    return D_800D18E0[index].character;
}

#pragma GLOBAL_ASM("asm/nonmatchings/main/main/func_80029274.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/main/func_800293D0.s")
