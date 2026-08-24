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
extern s32 D_800CF3D4;

/* PROVENANCE: field roles adapted from JFG src/level.c; Mickey layout is decisive. */
typedef struct LevelSummary {
    u8 type;
    s8 world;
    u8 objectFlag;
    u8 flags;
    u8 tune;
    u8 blur;
} LevelSummary;

extern LevelSummary *D_800CF3DC;
extern u32 func_800291F0(void);

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

    result = func_800291F0() - 1;
    if ((arg0 >= 0) && (arg0 < D_800CF3D4) && (result == 0) &&
        (((u32) D_800CF3DC[arg0].flags >> 5) != 0)) {
        result = 4;
    }
    return result;
}

#pragma GLOBAL_ASM("asm/nonmatchings/main/level/levelInit.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/level/levelTunePlay.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/level/levelUpdateColourCycling.s")

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

#pragma GLOBAL_ASM("asm/nonmatchings/main/level/levelGetName.s")

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

#pragma GLOBAL_ASM("asm/nonmatchings/main/level/levelGetPrevOfWorld.s")

#pragma GLOBAL_ASM("asm/nonmatchings/main/level/levelInitRegionFlags.s")
