#include "ultra64.h"
#include "game/gameVi.h"
#include "game/menu.h"
#include "game/sched_internal.h"

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
extern s32 D_8007A150;
extern s32 D_8007A158;
extern s32 D_8007A168;
extern s32 D_8007A1B0;
extern s32 D_8007A1BC;
extern s32 D_8007A1AC;
extern s32 D_8007A1C0;
extern s32 D_8007A13C;
extern s32 D_8007A140;
extern s32 D_8007A144;
extern s32 D_8007A12C;
extern s32 D_8007A130;
extern s32 D_8007A134;
extern s32 D_8007A138;
extern s32 D_8007A1A4;
extern s32 D_8007A1A8;
extern s32 D_8007A194;
extern s8 D_8007A1A0;
extern u32 D_8007A1CC;
extern s32 D_8007A1D4;
extern s32 D_8007A1EC;
extern s32 D_8007A1B4;
extern s32 D_8007A1D8;
extern s32 D_8007A200;
extern void *D_8007A204;
extern s32 D_8007A24C;
extern s32 D_8007A320;
extern u8 *D_8007A244;
extern s16 D_8007A250[];
extern s32 D_8007A258;
extern u8 D_8007BEF4;
extern u8 D_8007BEF8;
extern s8 D_8007BEF0;
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

typedef struct MainZBCheck {
    s16 x;
    s16 y;
    u8 pad4;
    s8 width;
    s8 height;
    s8 enabled;
} MainZBCheck;

extern MainGameEntry *D_800D18E0;
extern void *D_800D18E4;
extern u8 D_800D1928[];
extern OSMesgQueue D_800D18F8;
extern OSMesg D_800D18F4;
extern OSScClient D_800D18E8;
extern OSSched D_800CF5B8;
extern s32 D_800D1910;
extern s32 D_800D18C0;
extern s32 D_800D18C4;
extern s32 D_800D18C8;
extern s32 D_800D18CC;
extern s32 D_800D18D0;
extern s32 D_800D18D4;
extern s32 D_800D18D8;
extern s32 D_800D18DC;
extern MainZBCheck D_800CF538[];
extern u16 *D_800D2FAC;
extern s32 osTvType;
extern s32 levelNGetType(s32 level);
extern void func_80028EFC(MainCharacterState *, s32, s32);
extern void mainChangeLevel(s32, s32, s32, s32, s32, s32);
extern void func_80005548(s32);
extern void func_80028DE4(s32, s32, s32, s32, s32, s32);
extern void func_8003A544(s32);
extern void joyResetMap(void);
extern void func_8004978C(s32, s32, s32);
extern s32 func_80049864(s32);
extern void func_800498FC(s32, u32, u32, s32, s32, s32, s32);
extern s32 TrapDanglingJump();
extern s32 joyRead(s32, s32);
extern void mainInitGame(void);
extern void mainPreNMI(void);
extern void func_80026FB4(void);
extern void func_80021290(void);
extern void func_80001BC4(void);
extern void func_800339B4(void);
extern void rumbleKill(s32);
extern void rumbleTick(s32);
extern void osSetTime(OSTime);
extern OSTime osGetTime(void);
extern u16 joyGetButtons(s32);
extern s32 func_8003A24C(void);
extern void func_8003A260(s32);
extern void func_8003A2C8(s32);
extern s32 func_8003A408(void);
extern void func_8003A41C(s32);
extern void mainFrontInit(s32, s32, s32);
extern void RevealReturnAddresses(void);
extern void mmInit(void);
extern void func_8004D750(void);
extern void *func_8002B280(s32, s32);
extern void osCreateScheduler(OSSched *, void *, s32, u8, u8);
extern void osScAddClient(OSSched *, OSScClient *, OSMesgQueue *, u8);
extern void piInit(void);
extern void rcpInit(OSSched *);
extern void runlinkInit(void);
extern void func_80032338(s32);
extern s32 func_80021C5C(s32);
extern void func_80021F68(s32, s32 *, s32 *, s32 *, s32 *);
extern f32 rainDensity(void);
extern s32 runlinkIsModuleLoaded(s32);
extern void func_8004A0F0(void);
extern void func_8004A51C(void);
extern void func_8004AD34(void);
extern s32 D_800D40E4;

#ifdef NON_MATCHING
#pragma weak mainCPUeffectsRainDraw = TrapDanglingJump
extern void mainCPUeffectsRainDraw(void *, s32, s32, f32, f32);
#endif

#ifdef NON_MATCHING
/*
 * Plateau: nine source/expression hypotheses preserve all 66 target opcodes,
 * the 0x108 boundary, -0x30 frame and exact relocation layout. The best has
 * 20 register-operand differences, first at +0x24 where the target loads its
 * comparison constant before the outer countdown. The remaining differences
 * are the temp-FIFO choices in the byte-patch sequence. The complete flag
 * lattice was unchanged; a canonical-MIPS-II bounded permuter batch improved
 * its score from 225 to 120 without reaching identity.
 */
void RevealReturnAddresses(void) {
    s32 outer;
    u8 **returnAddress;
    u8 *scan;
    s32 inner;
    u16 patched;

    outer = 4;
    do {
        returnAddress = &D_8007A244;
        outer = 4;
        do {
            scan = *returnAddress;
            inner = 0x3F;
            do {
                if (((*(u32 *) scan >> 26) == 9) &&
                    (*(u16 *) (scan + 2) == 0x666)) {
                    scan[0] = scan[0] & 0xFF03;
                    *(u16 *) scan = (patched = *(u16 *) scan | 0x3E0);
                    scan[2] = ((patched << 1) << 2) |
                              (scan[2] & 0xFF07);
                    *(u16 *) (scan + 2) &= 0xF83F;
                    scan[1] = scan[1] & 0xFFE0;
                    scan[3] = (scan[3] & 0xFFC0) | 0x25;
                    osWritebackDCache(scan, 4);
                    osInvalICache(scan, 4);
                    break;
                }
                scan += 4;
            } while (inner--);
            returnAddress--;
        } while (outer--);
    } while (0);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/main/RevealReturnAddresses.s")
#endif

#ifdef NON_MATCHING
/*
 * PROVENANCE: body adapted from JFG src/main.c; Mickey byte identity is decisive.
 *
 * Plateau: the JFG RAM_END spelling reproduces all 50 linked instruction
 * words and the 0xC8 boundary, but omits the target assembly's D_803FFFFC
 * HI16/LO16 pair at +0x18/+0x28. Symbolic pointer and array spellings emit an
 * extra address instruction, which moves the aligned epilogue and adds eight
 * words. The complete resident flag lattice leaves this result unchanged.
 */
void mainThread(void *unused) {
    s32 i;

    mainInitGame();
    if (osTvType == 0) {
        i = 0;
        while (1) {
            ((volatile u32 *) 0x80400000)[--i] = 0;
        }
    }
    D_8007A200 = 1;
    TrapDanglingJump();
    D_8007A1CC = joyRead(D_8007A1CC, 0);
    D_8007A1B4 = 1;
    D_8007A1D8 = 0;
    D_8007A1BC = 6;
    mainChangeLevel(0, D_8007A154, 0, 0, 1, 0);
    while (1) {
        mainPreNMI();
        func_80026FB4();
        func_80021290();
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/main/mainThread.s")
#endif

/* PROVENANCE: body adapted from JFG src/main.c; Mickey byte identity is decisive. */
s32 mainResetPressed(void) {
    if (D_800D1910 == 0) {
        D_800D1910 = (osRecvMesg(&D_800D18F8, NULL, OS_MESG_NOBLOCK) + 1) != 0;
    }
    return D_800D1910;
}

/* PROVENANCE: body adapted from JFG src/main.c; Mickey byte identity is decisive. */
void mainPreNMI(void) {
    OSTime time;

    if (mainResetPressed()) {
        if (D_8007A1B4 != 0) {
            func_80001BC4();
            osSetTime(0);
            time = osGetTime();
            while (time < 2300000) {
                time = osGetTime();
            }
            __osSpSetStatus(0xAAAA82);
            osDpSetStatus(0x1D6);
            func_800339B4();
            rumbleKill(1);
            rumbleTick(2);
            rumbleTick(2);
            rumbleTick(2);
        }
        while (1) {
        }
    }
}

/*
 * PROVENANCE: body adapted from Jet Force Gemini src/main.c::mainInitGame;
 * Mickey's subsystem calls, storage and byte identity are decisive.
 */
void mainInitGame(void) {
    s32 viMode;

    if (osTvType == 0) {
        viMode = 0xE;
    } else if (osTvType == 1) {
        viMode = 0;
    } else if (osTvType == 2) {
        viMode = 0x1C;
    }
    osCreateScheduler(&D_800CF5B8, &D_800D18C0, 13, viMode, 1);
    RevealReturnAddresses();
    mmInit();
    func_8004D750();
    D_8007A204 = func_8002B280(0x40, 0x7F7F7FFF);
    D_8007A1AC = 0;
    osCreateMesgQueue(&D_800D18F8, &D_800D18F4, 1);
    osScAddClient(&D_800CF5B8, &D_800D18E8, &D_800D18F8, 3);
    D_800D1910 = 0;
    D_8007A320 = 1;
    D_8007A1B4 = 0;
    viInit(&D_800CF5B8);
    mainPreNMI();
    piInit();
    mainPreNMI();
    rcpInit(&D_800CF5B8);
    mainPreNMI();
    runlinkInit();
    mainPreNMI();
    TrapDanglingJump();
    mainPreNMI();
    func_80032338(0x12);
    D_8007A320 = 0;
}

#pragma GLOBAL_ASM("asm/nonmatchings/main/main/func_80026FB4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/main/func_80027628.s")

s32 mainAddZBCheck(s32 x, s32 y, s32 radius) {
    s32 result;
    s32 x1;
    s32 y1;
    s32 x2;
    s32 y2;
    s32 screenWidth;
    s32 screenHeight;

    viGetCurrentSize(&screenWidth, &screenHeight);
    result = -1;
    if (D_8007A24C < 8) {
        x2 = x + radius;
        y2 = y + radius;
        if ((x2 >= 0) && (y2 >= 0)) {
            x1 = x - radius;
            y1 = y - radius;
            if ((x1 < screenWidth) && (y1 < screenHeight)) {
                if (x1 < 0) {
                    x1 = 0;
                }
                if (y1 < 0) {
                    y1 = 0;
                }
                if (x2 >= screenWidth) {
                    x2 = screenWidth - 1;
                }
                if (y2 >= screenHeight) {
                    y2 = screenHeight - 1;
                }
                result = D_8007A24C++;
                D_800CF538[result].x = x1;
                D_800CF538[result].y = y1;
                D_800CF538[result].width = (x2 - x1) + 1;
                D_800CF538[result].height = (y2 - y1) + 1;
            }
        }
    }
    return result;
}

#ifdef NON_MATCHING
/*
 * Plateau: the Mickey-derived nested countdown loops compile to 60 of the
 * target's 63 instructions with the exact -0x48 frame and screen-size stack
 * slots. The first mismatch is +0x24: IDO schedules the outer counter before
 * the target's D_8007A24C/D_800D2FAC LO16 pair. The target also retains three
 * dead-looking loop-register copies that the natural C removes. The complete
 * resident flag lattice and a bounded permuter run did not recover that
 * spelling.
 */
void mainUpdateZBCheck(void) {
    MainZBCheck *check;
    s32 i;
    s32 enabled;
    s32 rows;
    s32 screenWidth;
    s32 screenHeight;
    s32 columns;
    s32 width;
    u8 *row;
    u16 *pixel;

    viGetCurrentSize(&screenWidth, &screenHeight);
    check = D_800CF538;
    i = 7;
    do {
        enabled = 1;
        if (D_8007A24C > 0) {
            D_8007A24C--;
            rows = check->height;
            row = (u8 *) D_800D2FAC +
                  (((check->y * screenWidth) + check->x) * 2);
            if (rows != 0) {
                rows--;
                width = check->width;
                do {
                    columns = width;
                    if (width != 0) {
                        columns--;
                        pixel = (u16 *) (row + (columns * 2));
                        do {
                            if ((*pixel & 0xFFFC) == 0xFFFC) {
                                enabled = 0;
                                goto nextCheck;
                            }
                            pixel--;
                        } while (columns--);
                    }
                    row += screenWidth * 2;
                } while (rows--);
            }
        }
nextCheck:
        check->enabled = enabled;
        check++;
    } while (i--);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/main/mainUpdateZBCheck.s")
#endif

/* PROVENANCE: body adapted from JFG src/main.c; Mickey byte identity is decisive. */
s8 mainGetZBCheck(s32 arg0) {
    if ((arg0 < 0) || (arg0 >= 8)) {
        return 1;
    }
    return D_800CF53F[arg0 * 8];
}

#ifdef NON_MATCHING
/*
 * PROVENANCE: function role adapted from JFG src/main.c::mainCPUeffects. JFG
 * retains assembly, so the body is reconstructed from Mickey-only control-flow
 * and call evidence; Mickey byte identity is decisive.
 *
 * Plateau: ten type, expression, storage and statement-grouping hypotheses
 * preserve all 85 target opcodes, the 0x154 boundary and the -0x40 frame. The
 * best differs in ten temp-FIFO register operands, first at +0x48, and its
 * typed overlay-call alias retains a different relocation identity at +0xD8.
 * The complete resident flag lattice does not improve either residual.
 */
void mainCPUeffects(u16 *framebuffer, s32 unused) {
    s32 screenWidth;
    u32 screenHeight;
    s32 x1;
    s32 y1;
    s32 x2;
    s32 y2;

    viGetCurrentSize(&screenWidth, (s32 *) &screenHeight);
    if (func_80021C5C(0) != 0) {
        func_80021F68(0, &x1, &y1, &x2, &y2);
        screenHeight = (y2 - y1) + 1;
        framebuffer += y1 * screenWidth;
    }
    if ((func_80049864(4) == 0) &&
        (runlinkIsModuleLoaded(0xF) != 0) &&
        (TrapDanglingJump() != 0)) {
        if ((D_800D40E4 > 0) && (screenHeight >= D_800D40E4)) {
            mainCPUeffectsRainDraw(framebuffer, screenWidth, D_800D40E4,
                                   256.0f, rainDensity());
        }
        D_800D40E4 = screenHeight;
    }
    func_8004AD34();
    if ((runlinkIsModuleLoaded(0x21) != 0) &&
        (func_80049864(4) == 0)) {
        TrapDanglingJump();
    }
    if (func_80049864(4) == 0) {
        func_8004A51C();
    } else {
        func_8004A0F0();
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/main/mainCPUeffects.s")
#endif

void mainSetGameWindow(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4,
                       s32 arg5, s32 arg6, s32 arg7, s32 arg8) {
    D_800D18C0 = arg0;
    D_800D18C4 = arg1;
    D_800D18C8 = arg2;
    D_800D18CC = arg3;
    D_800D18D0 = arg4;
    D_800D18D4 = arg5;
    D_800D18D8 = arg6;
    D_800D18DC = arg7;
    if (arg8 < 0) {
        D_8007A140 = -arg8;
        D_8007A144 = 1;
    } else {
        D_8007A140 = arg8;
        D_8007A144 = 0;
    }
    D_8007A13C = D_8007A140;
}

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

void func_80027EC0(s32 arg0) {
    s32 i;
    s32 buttons;

    i = 0;
    buttons = 0;
    do {
        buttons |= joyGetButtons(i) & 0x1000;
        i++;
    } while (i != 4);
    if (buttons != 0) {
        D_8007BEF0 = 1;
    }
    D_8007A1D8++;
    if (D_8007A1D8 >= 10) {
        func_8003A260(func_8003A24C());
        func_8003A2C8(frontGetScreenMode());
        frontSetWideAdjust(frontGetWideAdjust());
        func_8003A41C(func_8003A408());
        frontSetSfxVolume(frontGetSfxVolume());
        frontSetBgmVolume(frontGetBgmVolume());
        frontStoreScreenMode();
        mainFrontInit(2, 0, 0);
    }
}

#pragma GLOBAL_ASM("asm/nonmatchings/main/main/func_80027FB8.s")

void func_800282C8(void) {
    if (D_8007A194 == 0) {
        mainChangeLevel(D_8007A148, D_8007A150, D_8007A158, D_8007A168, 0, 0);
        if (func_80049864(4) == 0) {
            D_8007A1A0 = 0;
            func_800498FC(4, 0x3EAE147B, 0xBF800000, 0xFF, 0xFF, 0xFF, 0);
            func_8004978C(4, 1, 1);
        }
        D_8007A194 = 0x1E;
    }
}

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

/* PROVENANCE: function role adapted from JFG src/main.c order and body. */
void mainTitlePageInit(s32 arg0) {
    D_8007A1BC = 1;
    D_8007A1C0 = 1;
    func_8003A544(0);
    mainChangeLevel(D_8007A250[D_8007A258], 0, 0, 3, 1, 0);
    mainChangeCameras(1);
    func_80028DE4(6, 6, 0, 2, 0, 0);
    func_80005548(6);
    joyResetMap();
    D_8007A258 = (D_8007A258 + 1) & 3;
}

void func_80028DE4(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5) {
    D_8007A174 = arg0;
    D_8007A178 = arg1;
    D_8007A17C = arg2;
    D_8007A180 = arg3;
    D_8007A184 = arg4;
    D_8007A188 = arg5;
    D_8007A18C = 1;
}

/* PROVENANCE: body adapted from JFG src/main.c; Mickey byte identity is decisive. */
void mainFrontInit(s32 arg0, s32 arg1, s32 arg2) {
    D_8007A1BC = 1;
    D_8007A1C0 = 1;
    if (D_8007A1AC == 0) {
        D_8007A1AC = 1;
    }
    mainChangeLevel(arg1, D_8007A154, 0, arg0, 1, 0);
}

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

void func_80028F3C(void) {
}

void func_80028F44(void) {
}

void func_80028F4C(void) {
}

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

#ifdef NON_MATCHING
/*
 * PROVENANCE: structural comparison uses Jet Force Gemini
 * src/overlays/o3/overlay_3.c::GetSmoothAcceleration; JFG retains assembly,
 * so this body is reconstructed from Mickey-only control-flow evidence.
 *
 * Plateau: nine control-flow, parameter and register-lifetime hypotheses
 * reproduce the target's 87-instruction boundary and -0x10 frame. The best
 * differs in 42 words, first at +0x8: IDO moves the first float argument
 * before the saved-register store, colors the long-lived float webs
 * differently and reshapes the negative-velocity return path. The complete
 * resident flag lattice leaves this result unchanged.
 */
f32 func_80029274(s32 arg0, f32 arg1, f32 arg2) {
    f32 temp_f0;
    f32 temp_f16;
    f32 temp_f12;
    f32 temp_f16_2;
    f32 var_f0;
    f32 var_f12;
    f32 var_f2;
    register f32 var_f14;
    s32 temp_v0;

    var_f14 = arg1;
    temp_v0 = arg0 < 0;
    var_f0 = 0.0f;
    var_f2 = 0.0f;
    if (temp_v0 != 0) {
        arg0 = -arg0;
        var_f14 = -var_f14;
    }
    if (var_f14 < 0.0f) {
        if (temp_v0 != 0) {
            return -arg2;
        }
        return arg2;
    }
    temp_f12 = (f32) arg0;
    do {
        var_f2 += arg2;
        var_f0 += var_f2;
    } while ((var_f0 + var_f2) < temp_f12);
    if ((temp_f12 <= arg2) && (var_f14 <= arg2) &&
        (((arg0 >= 0) && (var_f14 >= 0.0f)) ||
         ((arg0 <= 0) && (var_f14 <= 0.0f)))) {
        var_f12 = 0.0f;
    } else {
        temp_f0 = var_f2 - arg2;
        temp_f16 = var_f14 + arg2;
        if (temp_f16 <= temp_f0) {
            var_f12 = temp_f16;
        } else {
            temp_f16_2 = var_f14 - arg2;
            if (temp_f16_2 < var_f2) {
                var_f12 = temp_f0;
                if (var_f2 == arg2) {
                    goto useAcceleration;
                }
            } else {
                var_f12 = temp_f16_2;
                if (temp_f16_2 == 0.0f) {
useAcceleration:
                    var_f12 = arg2;
                }
            }
        }
    }
    if (temp_v0 != 0) {
        var_f12 = -var_f12;
        var_f14 = -var_f14;
    }
    return var_f12 - var_f14;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/main/func_80029274.s")
#endif

#pragma GLOBAL_ASM("asm/nonmatchings/main/main/func_800293D0.s")
