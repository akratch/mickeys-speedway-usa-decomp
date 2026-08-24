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
extern OSMesgQueue D_800CF340;

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
extern s32 func_8003A550(void);

/* PROVENANCE: body adapted from JFG src/joy.c; Mickey byte identity is decisive. */
OSMesgQueue *joyMessageQ(void) {
    return &D_800CF340;
}

#pragma GLOBAL_ASM("asm/nonmatchings/main/joy/joyInit.s")

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

#pragma GLOBAL_ASM("asm/nonmatchings/main/joy/joyCreateMap.s")

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

#pragma GLOBAL_ASM("asm/nonmatchings/main/joy/joyGetStickX.s")

/* PROVENANCE: body adapted from JFG src/joy.c; Mickey byte identity is decisive. */
s8 joyGetAbsX(s32 player) {
    return D_800CF372[D_800CF3B0[player] * 6];
}

#pragma GLOBAL_ASM("asm/nonmatchings/main/joy/joyGetStickY.s")

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

#pragma GLOBAL_ASM("asm/nonmatchings/main/joy/arithmeticFunction.s")

/* PROVENANCE: body adapted from JFG src/joy.c; Mickey byte identity is decisive. */
s32 joyCharVal(void) {
    return 1;
}
