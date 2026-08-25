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
typedef struct LevelHeaderSummarySource LevelHeaderSummarySource;
extern LevelHeaderSummarySource *D_800CF3C8;
extern s32 *D_800CF3C0;
extern s32 D_800CF3D4;
extern s32 D_800CF3D8;
extern s32 D_800CF3E0[16];
extern u8 **D_800CF3D0;
extern u8 *D_8007A0D0;
extern u8 D_8007BF08;

/* PROVENANCE: field roles adapted from JFG src/level.c; Mickey layout is decisive. */
typedef struct LevelSummary {
    u8 type;
    s8 world;
    u8 objectFlag;
    u8 gfxIndex : 3;
    u8 screenMode : 2;
    u8 region : 3;
    u8 tune;
    u8 blur;
} LevelSummary;

typedef struct LevelHeaderColour {
    u8 pad0[0x94];
    s16 weatherType[7];
} LevelHeaderColour;

struct LevelHeaderSummarySource {
    u8 pad0[0x20];
    s8 world;
    u8 pad21[2];
    s8 gfxIndex;
    u8 pad24[0x5F];
    s8 type;
    u8 pad84[0xA];
    u8 tune;
    u8 pad8F[0x3D];
    s8 screenMode;
    u8 region;
    u8 padCE[0x39];
    u8 blur;
    u8 pad108[0x28];
};

extern LevelSummary *D_800CF3DC;
extern s32 mainGetNumberOfCameras(void);
extern void func_80000510(u8);
extern void amTuneResetFade(void);
extern void func_80000B48(u16);
extern void amTuneResetChls(void);
extern void amTuneStop(void);
extern u8 amTuneGetSeqNo(void);
extern void func_80036AB0(void *, s32);
extern s32 *piRomLoad(s32);
extern void piRomLoadSection(s32, void *, s32, s32);
extern void mmFree(void *);
extern void *func_8002B280(s32, s32);
extern u8 *align4(u8 *);

#ifdef NON_MATCHING
/*
 * PROVENANCE: body adapted from JFG src/level.c; Mickey byte identity is decisive.
 *
 * Plateau: the candidate has the target's 259 instructions, -0x58 frame and
 * opcode schedule, but three register operands differ, first at +0x13C. The
 * zero-count loop also binds D_800CF3E0 where the target object names
 * D_800CF420 at its HI16/LO16 relocation pair. The resident flag lattice and
 * a bounded MIPS II permuter batch did not close either residual.
 */
void levelGetCounts(void) {
    s32 i;
    s32 count;
    LevelHeaderSummarySource *header;
    u8 *nameData;

    header = func_8002B280(sizeof(LevelHeaderSummarySource), 0x8F);
    D_800CF3C0 = piRomLoad(0x1E);

    for (i = 0; i != 16; i++) { D_800CF3E0[i] = 0;
    }

    D_800CF3D4 = 0;
    while (D_800CF3C0[D_800CF3D4] != -1) {
        D_800CF3D4++;
    }
    D_800CF3D4--;

    D_800CF3DC = func_8002B280(D_800CF3D4 * sizeof(LevelSummary), 0x8F);
    D_800CF3D8 = -1;
    D_800CF3C8 = header;
    for (i = 0; i < D_800CF3D4; i++) {
        piRomLoadSection(0x1F, D_800CF3C8, D_800CF3C0[i], sizeof(LevelHeaderSummarySource));
        if (D_800CF3C8->world > D_800CF3D8) {
            D_800CF3D8 = D_800CF3C8->world;
        }
        if ((D_800CF3C8->type >= 0) && (D_800CF3C8->type < 16)) { D_800CF3E0[D_800CF3C8->type]++;
        }
        D_800CF3DC[i].type = D_800CF3C8->type;
        D_800CF3DC[i].world = D_800CF3C8->world;
        D_800CF3DC[i].gfxIndex = D_800CF3C8->gfxIndex;
        D_800CF3DC[i].screenMode = D_800CF3C8->screenMode;
        D_800CF3DC[i].region = D_800CF3C8->region;
        D_800CF3DC[i].tune = D_800CF3C8->tune;
        D_800CF3DC[i].blur = D_800CF3C8->blur;
    }
    D_800CF3D8++;
    mmFree(D_800CF3C0);
    mmFree(header);

    D_8007A0D0 = func_8002B280(0x20, 0x8F);
    D_800CF3C0 = piRomLoad(0x22);
    i = 0;
    while (D_800CF3C0[i] != -1) {
        i++;
    }
    i--;
    count = D_800CF3C0[i] - D_800CF3C0[0];
    nameData = func_8002B280((s32) align4((u8 *) count) + (i * 4), 0x8F);
    D_800CF3D0 = (u8 **) ((u32) nameData + (s32) align4((u8 *) count));
    piRomLoadSection(0x23, nameData, 0, count);
    for (count = 0; count < i; count++) {
        D_800CF3D0[count] = nameData + D_800CF3C0[count];
    }
    mmFree(D_800CF3C0);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/level/levelGetCounts.s")
#endif

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
        return D_800CF3DC[arg0].region;
    }
    return 0;
}

/* PROVENANCE: body adapted from JFG src/level.c; Mickey byte identity is decisive. */
s32 levelGetScreenMode(s32 arg0) {
    if ((arg0 >= 0) && (arg0 < D_800CF3D4)) {
        return D_800CF3DC[arg0].screenMode;
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
        (D_800CF3DC[arg0].gfxIndex != 0)) {
        result = 4;
    }
    return result;
}

#pragma GLOBAL_ASM("asm/nonmatchings/main/level/levelInit.s")

/* PROVENANCE: body adapted from JFG src/level.c; Mickey byte identity is decisive. */
void levelTunePlay(void) {
    if (((u8 *) D_800CF3C8)[0x8E] != 0) {
        if (((u8 *) D_800CF3C8)[0x8E] != amTuneGetSeqNo()) {
            amTuneResetChls();
            func_80000510(((u8 *) D_800CF3C8)[0x8E]);
            amTuneResetFade();
            func_80000B48(*(u16 *) &((u8 *) D_800CF3C8)[0x90]);
        }
    } else {
        amTuneStop();
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
    return ((u8 *) D_800CF3C8)[0x83];
}

/* PROVENANCE: body adapted from JFG src/level.c; Mickey byte identity is decisive. */
u8 levelGetCamera(void) {
    if (D_800CF3C8 != NULL) {
        return ((u8 *) D_800CF3C8)[0xE6];
    }
    return 0;
}

/* PROVENANCE: body adapted from JFG src/level.c; Mickey byte identity is decisive. */
u8 *levelGetLevel(void) {
    return (u8 *) D_800CF3C8;
}

/* PROVENANCE: body adapted from JFG src/level.c; Mickey byte identity is decisive. */
u8 *levelGetName(s32 arg0) {
    *D_8007A0D0 = 0;
    if (arg0 < D_800CF3D4) {
        D_800CF3C0 = piRomLoad(0x1E);
        if (D_800CF3C0 != NULL) {
            piRomLoadSection(0x1F, D_8007A0D0, D_800CF3C0[arg0], 0x20);
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
    if (((s8) ((u8 *) D_800CF3C8)[0x83] == 0) && (D_8007BF08 != 0)) {
        return 1;
    }
    return 0;
}
