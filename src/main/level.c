#include "ultra64.h"

/*
 * Resident level lifecycle and metadata, ROM 0x263F0-0x27760.
 *
 * PROVENANCE: the TU attribution and function names are adapted from Jet
 * Force Gemini's published src/level.c and built src/level.c.o, a permitted
 * public retail-derived decomp under docs/CLEANROOM.md. Mickey's ordered call
 * graph independently establishes the correspondence (tier B).
 */

extern s32 D_800CF3C4;
extern u8 D_800CF420[];
extern u8 *D_800CF3C8;
extern s32 *D_800CF3C0;
extern s32 D_800CF3D4;
extern u8 *D_8007A0D0;
extern u8 D_8007BF08;

/* PROVENANCE: field roles adapted from JFG src/level.c; Mickey layout is decisive. */
typedef struct LevelSummary {
    u8 type;
    s8 world;
    u8 objectFlag;
    u8 flags;
    u8 tune;
    u8 blur;
} LevelSummary;

typedef struct LevelHeaderColour {
    u8 pad0[0x94];
    s16 weatherType[7];
} LevelHeaderColour;

extern LevelSummary *D_800CF3DC;
extern s32 mainGetNumberOfCameras(void);
extern void func_80000510(u8);
extern void func_80000730(void);
extern void func_80000B48(u16);
extern void func_80000C38(void);
extern void func_80000CEC(void);
extern u8 func_80000D54(void);
extern void func_80036AB0(void *, s32);
extern s32 *func_8002E148(s32);
extern void func_8002E2E0(s32, void *, s32, s32);
extern void mmFree(void *);

#pragma GLOBAL_ASM("asm/nonmatchings/main/level/levelGetCounts.s")

/* PROVENANCE: body adapted from JFG src/level.c; Mickey byte identity is decisive. */
s32 levelNGetType(s32 arg0) {
    if ((arg0 >= 0) && (arg0 < D_800CF3D4)) {
        return D_800CF3DC[arg0].type;
    }
    return -1;
}

/* PROVENANCE: body adapted from JFG src/level.c; Mickey byte identity is decisive. */
s32 levelGetTune(s32 arg0) {
    if ((arg0 >= 0) && (arg0 < D_800CF3D4)) {
        return D_800CF3DC[arg0].tune;
    }
    return -1;
}

/* PROVENANCE: body adapted from JFG src/level.c; Mickey byte identity is decisive. */
s32 levelGetWorld(s32 arg0) {
    if ((arg0 >= 0) && (arg0 < D_800CF3D4)) {
        return D_800CF3DC[arg0].world;
    }
    return 0;
}

/* PROVENANCE: body adapted from JFG src/level.c; Mickey byte identity is decisive. */
s32 levelGetRegionNo(s32 arg0) {
    if ((arg0 >= 0) && (arg0 < D_800CF3D4)) {
        return D_800CF3DC[arg0].flags & 7;
    }
    return 0;
}

/* PROVENANCE: body adapted from JFG src/level.c; Mickey byte identity is decisive. */
s32 levelGetScreenMode(s32 arg0) {
    if ((arg0 >= 0) && (arg0 < D_800CF3D4)) {
        return ((u32) D_800CF3DC[arg0].flags << 27) >> 30;
    }
    return 0;
}

/* PROVENANCE: body adapted from JFG src/level.c; Mickey byte identity is decisive. */
s32 levelGetBlurEffect(s32 arg0) {
    if ((arg0 >= 0) && (arg0 < D_800CF3D4)) {
        return D_800CF3DC[arg0].blur;
    }
    return 0;
}

/* PROVENANCE: body adapted from JFG src/level.c; Mickey byte identity is decisive. */
u32 levelGetGfxIndex(s32 arg0) {
    u32 result;

    result = mainGetNumberOfCameras() - 1;
    if ((arg0 >= 0) && (arg0 < D_800CF3D4) && (result == 0) &&
        (((u32) D_800CF3DC[arg0].flags >> 5) != 0)) {
        result = 4;
    }
    return result;
}

#pragma GLOBAL_ASM("asm/nonmatchings/main/level/levelInit.s")

/* PROVENANCE: body adapted from JFG src/level.c; Mickey byte identity is decisive. */
void levelTunePlay(void) {
    if (D_800CF3C8[0x8E] != 0) {
        if (D_800CF3C8[0x8E] != func_80000D54()) {
            func_80000C38();
            func_80000510(D_800CF3C8[0x8E]);
            func_80000730();
            func_80000B48(*(u16 *) &D_800CF3C8[0x90]);
        }
    } else {
        func_80000CEC();
    }
}

/* PROVENANCE: body adapted from JFG src/level.c; Mickey byte identity is decisive. */
void levelUpdateColourCycling(s32 arg0) {
    s32 i;

    for (i = 0; i < 7; i++) {
        if (((LevelHeaderColour *) D_800CF3C8)->weatherType[i] != -1) {
            func_80036AB0(&D_800CF420[i * 16], arg0);
        }
    }
}

/* PROVENANCE: body adapted from JFG src/level.c; Mickey byte identity is decisive. */
u8 *levelGetColourCycling(void) {
    return D_800CF420;
}

/* PROVENANCE: body adapted from JFG src/level.c; Mickey byte identity is decisive. */
s32 levelGetNumber(void) {
    return D_800CF3C4;
}

/* PROVENANCE: body adapted from JFG src/level.c; Mickey byte identity is decisive. */
u8 levelGetType(void) {
    return D_800CF3C8[0x83];
}

/* PROVENANCE: body adapted from JFG src/level.c; Mickey byte identity is decisive. */
u8 levelGetCamera(void) {
    if (D_800CF3C8 != NULL) {
        return D_800CF3C8[0xE6];
    }
    return 0;
}

/* PROVENANCE: body adapted from JFG src/level.c; Mickey byte identity is decisive. */
u8 *levelGetLevel(void) {
    return D_800CF3C8;
}

/* PROVENANCE: body adapted from JFG src/level.c; Mickey byte identity is decisive. */
u8 *levelGetName(s32 arg0) {
    *D_8007A0D0 = 0;
    if (arg0 < D_800CF3D4) {
        D_800CF3C0 = func_8002E148(0x1E);
        if (D_800CF3C0 != NULL) {
            func_8002E2E0(0x1F, D_8007A0D0, D_800CF3C0[arg0], 0x20);
            mmFree(D_800CF3C0);
        }
    }
    return D_8007A0D0;
}

#pragma GLOBAL_ASM("asm/nonmatchings/main/level/levelFreeAll.s")

/* PROVENANCE: body adapted from JFG src/level.c; Mickey byte identity is decisive. */
s32 levelGetNextOfWorld(s32 arg0, s8 arg1) {
    s32 next;

    next = arg0 + 1;
    if (next >= D_800CF3D4) {
        next = 0;
    }
    while ((next != arg0) && (arg1 != D_800CF3DC[next].world)) {
        next++;
        if (next >= D_800CF3D4) {
            next = 0;
        }
    }
    return next;
}

/* PROVENANCE: body adapted from JFG src/level.c; Mickey byte identity is decisive. */
s32 levelGetPrevOfWorld(s32 arg0, s8 arg1) {
    s32 prev;

    prev = arg0 - 1;
    if (prev < 0) {
        prev = D_800CF3D4 - 1;
    }
    while ((prev != arg0) && (arg1 != D_800CF3DC[prev].world)) {
        prev--;
        if (prev < 0) {
            prev = D_800CF3D4 - 1;
        }
    }
    return prev;
}

s32 levelInitRegionFlags(void) {
    if (((s8) D_800CF3C8[0x83] == 0) && (D_8007BF08 != 0)) {
        return 1;
    }
    return 0;
}
