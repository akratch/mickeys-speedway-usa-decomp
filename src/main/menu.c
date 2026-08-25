/*
 * Resident front-end menu -- ROM 0x39350-0x3B1A0 (VRAM 0x80038750).
 *
 * The translation-unit identity and function crosswalk come from Jet Force
 * Gemini's public decompilation of the same Rare engine. The boundary evidence
 * and provenance are recorded in docs/modules.md section 3.4. Functions stay
 * under GLOBAL_ASM until their C compiles to Mickey's bytes exactly.
 *
 * Flags: -O2 -mips2 -32, via the shared src/main rule.
 */

#include "PR/ultratypes.h"
#include "game/menu.h"

/* PROVENANCE: base layout adapted from JFG's public decomp,
 * src/menu.h::Resbitfield; twoPlayerSplit is Mickey-derived from its paired
 * getter and byte-preserving setter. */
typedef struct MenuScreenModeBits {
    u32 unused : 1;
    u32 modeBit0 : 1;
    u32 modeBit1 : 1;
    u32 twoPlayerSplit : 1;
    u32 rest : 28;
} MenuScreenModeBits;

extern s8 D_800D312B;
extern s8 D_800D3050;
extern MenuScreenModeBits D_800D3128;
extern u8 D_8007C08C;
extern u8 D_8007C090;
extern s32 D_8007C098;
extern s16 D_8007BF70;
extern u16 D_800D312C;
extern u16 D_800D312E;
extern void amTuneStop(void);
extern void amTuneSetGlobalVolume(s32 volume);
extern s32 levelGetRegionNo(void);
extern s8 viGetWideAdjust(void);
extern void gsSndpSetGlobalVolume(s32 volume);
extern void viSetWideAdjust(s32 offset);

#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_80038750.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_80038878.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_800389CC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_80038BC4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_80038DAC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_80038E10.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_80038E1C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_80039278.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_80039380.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/frontDrawRectangle.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_800395D4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_8003968C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_80039720.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_80039A40.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_80039A9C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_80039B88.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_80039BE4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_80039D50.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/setupFrontEndObject.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_80039E34.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_8003A24C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_8003A260.s")
s32 frontGetScreenMode(void) {
    s32 mode;

    mode = 0;
    if (D_800D3128.modeBit0) {
        mode = 1;
    }
    if (D_800D3128.modeBit1) {
        mode |= 2;
    }
    return mode;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_8003A2C8.s")
/* PROVENANCE: adapted from JFG's public decomp, src/menu.c::frontStoreScreenMode. */
void frontStoreScreenMode(void) {
    D_8007C08C = D_8007C090;
}
/* PROVENANCE: adapted from JFG's public decomp, src/menu.c::frontRecallScreenMode. */
u8 frontRecallScreenMode(void) {
    return D_8007C08C;
}
s32 frontGetLevelScreenMode(void) {
    /* Mickey-derived control flow; JFG's body remains GLOBAL_ASM. */
    if (D_8007C090 == 1) {
        goto mode_one;
    }
    if (D_8007C090 == 2) {
        goto mode_two;
    }
    if (D_8007C090 != 3) {
        goto current_mode;
    }

    return 3;
mode_two:
    return levelGetRegionNo() | 2;
mode_one:
    return 1;
current_mode:
    return levelGetRegionNo();
}
/* PROVENANCE: adapted from JFG's public decomp, src/menu.c::frontGetWideAdjust. */
s8 frontGetWideAdjust(void) {
    return D_800D312B;
}
/* PROVENANCE: adapted from JFG's public decomp, src/menu.c::frontSetWideAdjust. */
void frontSetWideAdjust(s32 offset) {
    viSetWideAdjust(offset);
    D_800D312B = viGetWideAdjust();
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_8003A408.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_8003A41C.s")
/* Retain the anonymous spelling used by an unsplit resident assembly caller. */
#pragma weak func_8003A47C = frontGetSfxVolume
/* PROVENANCE: adapted from JFG's public decomp, src/menu.c::frontGetSfxVolume. */
u16 frontGetSfxVolume(void) {
    return D_800D312C;
}
/* Retain the anonymous spelling used by an unsplit resident assembly caller. */
#pragma weak func_8003A488 = frontSetSfxVolume
/* PROVENANCE: adapted from JFG's public decomp, src/menu.c::frontSetSfxVolume. */
void frontSetSfxVolume(s32 volume) {
    if (volume < 0) {
        volume = 0;
    }
    if (volume > 0x100) {
        volume = 0x100;
    }
    D_800D312C = volume;
    gsSndpSetGlobalVolume(volume);
}
/* Retain the anonymous spelling used by an unsplit resident assembly caller. */
#pragma weak func_8003A4C4 = frontGetBgmVolume
/* PROVENANCE: adapted from JFG's public decomp, src/menu.c::frontGetBgmVolume. */
u16 frontGetBgmVolume(void) {
    return D_800D312E;
}
/* Retain the anonymous spelling used by an unsplit resident assembly caller. */
#pragma weak func_8003A4D0 = frontSetBgmVolume
/* PROVENANCE: adapted from JFG's public decomp, src/menu.c::frontSetBgmVolume. */
void frontSetBgmVolume(s32 volume) {
    if (volume < 0) {
        volume = 0;
    }
    if (volume > 0x100) {
        volume = 0x100;
    }
    D_800D312E = volume;
    amTuneSetGlobalVolume(volume);
}
s32 frontGet2PlayerSplit(void) {
    s32 split;

    split = D_800D3128.twoPlayerSplit;
    return split;
}
/* Size-exact plateau: three register operands differ from +0x8; IDO assigns
 * the old-flag value chain two temporary registers earlier than the target. */
#pragma GLOBAL_ASM("asm/nonmatchings/main/menu/func_8003A520.s")
void func_8003A544(s32 value) {
    D_8007C098 = value;
}
s32 func_8003A550(void) {
    return D_8007C098;
}
void func_8003A55C(s32 value) {
    amTuneStop();
    D_800D3050 = value;
    D_8007BF70 = 0x78;
}
void func_8003A590(void) {
    D_8007BF70 = -1;
}
