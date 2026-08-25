#include "ultra64.h"

/*
 * Resident controller input, ROM 0x25C20-0x263F0.
 *
 * PROVENANCE: the TU attribution and function names are adapted from Jet
 * Force Gemini's published src/joy.c and built src/controller.c.o, a permitted
 * public retail-derived decomp under docs/CLEANROOM.md. Mickey's ordered call
 * graph independently establishes the correspondence (tier B).
 */

extern u16 D_8007A0C8;
extern s32 D_8007A0C0;
extern OSMesgQueue D_800CF340;
extern OSMesg D_800CF358;
extern OSMesg D_800CF35C;
extern OSContStatus D_800CF360[];

/* PROVENANCE: field roles adapted from JFG src/joy.c; Mickey layout is decisive. */
typedef struct JoyPad {
    u16 button;
    s8 stick_x;
    s8 stick_y;
    u8 errno;
    u8 unused;
} JoyPad;

extern JoyPad D_800CF370[];
extern s8 D_800CF372[];
extern s8 D_800CF373[];
extern u16 D_800CF3A0[];
extern u16 D_800CF3A8[];
extern u8 D_800CF3B0[];
extern u8 D_800CF3B4[];
extern u8 D_800CF3B8[];
extern s32 D_800CF3BC;
extern s32 func_8003A550(void);
extern void TrapDanglingJump();
extern s32 osContInit(OSMesgQueue *, u8 *, OSContStatus *);
extern s32 osContStartReadData(OSMesgQueue *);
extern void joyResetMap(void);

s8 joyClamp(s8 stickMag);

/* PROVENANCE: body adapted from JFG src/joy.c; Mickey byte identity is decisive. */
OSMesgQueue *joyMessageQ(void) {
    return &D_800CF340;
}

#ifdef NON_MATCHING
/*
 * PROVENANCE: body adapted from JFG src/joy.c; Mickey byte identity is decisive.
 *
 * Plateau: the donor-shaped candidate is exact through +0x118, then emits
 * 86 instructions versus the target's 83. Because D_800CF3B4 is external to
 * this split TU, IDO materializes four HI16/LO16 pairs for the final unrolled
 * clears. The target shares one HI16 and uses four distinct LO16 identities
 * D_800CF3B4..D_800CF3B7. Scalar, aggregate, pointer, and expression-chain
 * spellings either retained the three surplus instructions or disrupted the
 * otherwise exact controller loop. The full flag lattice was unchanged, and
 * a bounded two-worker canonical-MIPS-II permuter batch found no improvement
 * from its base score of 325.
 */
s32 joyInit(void) {
    s32 i;
    u8 bitpattern;

    osCreateMesgQueue(&D_800CF340, &D_800CF358, 1);
    osSetEventMesg(5, &D_800CF340, D_800CF35C);
    osContInit(&D_800CF340, &bitpattern, D_800CF360);
    osContStartReadData(&D_800CF340);
    joyResetMap();

    for (i = 0; i < 4; i++) {
        D_800CF3B4[i] = TRUE;
        D_800CF3B8[i] = FALSE;
    }

    D_800CF3BC = 0;
    for (i = 0; i < 4; i++) {
        if ((bitpattern & (1 << i)) && !(D_800CF360[i].errno & 8)) {
            D_800CF3B8[i] = TRUE;
            D_800CF3BC++;
        }
    }

    if (D_800CF3B8[0] != 0) {
        return 0;
    }
    for (i = 0; i < 4; i++) {
        D_800CF3B4[i] = FALSE;
    }
    D_8007A0C0 = 1;
    return -1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/joy/joyInit.s")
#endif

#pragma GLOBAL_ASM("asm/nonmatchings/main/joy/joyRead.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/joy/joyResetMap.s")

/* PROVENANCE: body adapted from JFG src/joy.c; Mickey byte identity is decisive. */
void joyDisable(s32 player) {
    D_800CF3B4[player & 3] = FALSE;
}

/* PROVENANCE: body adapted from JFG src/joy.c; Mickey byte identity is decisive. */
void joyEnable(s32 player) {
    D_800CF3B4[player & 3] = TRUE;
}

/* PROVENANCE: body adapted from JFG src/joy.c; Mickey byte identity is decisive. */
void joyCreateMap(s8 *activePlayers) {
    s32 i;
    s32 temp = 0;

    for (i = 0; i < 4; i++) {
        if (activePlayers[i]) {
            D_800CF3B0[temp++] = i;
        }
    }
    for (i = 0; i < 4; i++) {
        if (!activePlayers[i]) {
            D_800CF3B0[temp++] = i;
        }
    }
}

/* PROVENANCE: body adapted from JFG src/joy.c; Mickey byte identity is decisive. */
u8 joyGetController(s32 player) {
    return D_800CF3B0[player];
}

/* PROVENANCE: body adapted from JFG src/joy.c; Mickey byte identity is decisive. */
u16 joyGetButtons(s32 player) {
    if (func_8003A550() != 0) {
        return 0;
    }
    return D_800CF370[D_800CF3B0[player]].button;
}

/* PROVENANCE: body adapted from JFG src/joy.c; Mickey byte identity is decisive. */
u16 joyGetPressed(s32 player) {
    if (func_8003A550() != 0) {
        return 0;
    }
    return D_800CF3A0[D_800CF3B0[player]];
}

/* PROVENANCE: body adapted from JFG src/joy.c; Mickey byte identity is decisive. */
u16 joyGetReleased(s32 player) {
    if (func_8003A550() != 0) {
        return 0;
    }
    return D_800CF3A8[D_800CF3B0[player]];
}

/* PROVENANCE: body adapted from JFG src/joy.c; Mickey byte identity is decisive. */
s8 joyGetStickX(s32 player) {
    if (func_8003A550() != 0) {
        return 0;
    }
    return joyClamp(D_800CF372[D_800CF3B0[player] * 6]);
}

/* PROVENANCE: body adapted from JFG src/joy.c; Mickey byte identity is decisive. */
s8 joyGetAbsX(s32 player) {
    return D_800CF372[D_800CF3B0[player] * 6];
}

/* PROVENANCE: body adapted from JFG src/joy.c; Mickey byte identity is decisive. */
s8 joyGetStickY(s32 player) {
    if (func_8003A550() != 0) {
        return 0;
    }
    return joyClamp(D_800CF373[D_800CF3B0[player] * 6]);
}

/* PROVENANCE: body adapted from JFG src/joy.c; Mickey byte identity is decisive. */
s8 joyGetAbsY(s32 player) {
    return D_800CF373[D_800CF3B0[player] * 6];
}

/* PROVENANCE: body adapted from JFG src/joy.c; Mickey byte identity is decisive. */
s8 joyClamp(s8 stickMag) {
    if ((stickMag < 5) && (stickMag > -5)) {
        return 0;
    }
    if (stickMag > 0) {
        stickMag -= 5;
        if (stickMag > 65) {
            stickMag = 65;
        }
    } else {
        stickMag += 5;
        if (stickMag < -65) {
            stickMag = -65;
        }
    }
    return stickMag;
}

/* PROVENANCE: body adapted from JFG src/joy.c; Mickey byte identity is decisive. */
void joySetSecurity(void) {
    D_8007A0C8 = 0;
}

/* PROVENANCE: body adapted from JFG src/joy.c; Mickey byte identity is decisive. */
void arithmeticFunction(u8 *challenge, u8 *response) {
    osRecvMesg(&D_800CF340, NULL, OS_MESG_BLOCK);
    TrapDanglingJump(challenge, &D_800CF340);
    osRecvMesg(&D_800CF340, NULL, OS_MESG_BLOCK);
    TrapDanglingJump(&D_800CF340);
    osRecvMesg(&D_800CF340, NULL, OS_MESG_BLOCK);
    osContStartReadData(&D_800CF340);
    TrapDanglingJump(response);
}

/* PROVENANCE: body adapted from JFG src/joy.c; Mickey byte identity is decisive. */
s32 joyCharVal(void) {
    return 1;
}
