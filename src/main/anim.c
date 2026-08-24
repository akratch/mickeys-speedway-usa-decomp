/*
 * Resident animation/collision block -- ROM 0x50C00-0x58570
 * (VRAM 0x80050000-0x80057970).
 *
 * PROVENANCE -- names and structural comparisons in this file use Jet Force
 * Gemini's public decompilation, principally src/anim.c, src/hit.c, src/fmv.c,
 * and their declarations. JFG is a permitted published retail-derived decomp
 * under docs/CLEANROOM.md. Mickey's own ROM remains authoritative; the block
 * is kept under its existing 16-byte-aligned boundaries because no internal
 * whole-object boundary has yet been proved.
 *
 * Flags: -O2 -mips2 -32, inherited from the src/main/ build rule.
 */

#include "PR/ultratypes.h"
#include "game/anim.h"

/*
 * PROVENANCE: adapted from JFG's func_80076020_76C20. Mickey's globals and
 * final compiler output are independently established from Mickey's ROM.
 */
void func_80050000(s32 *stream) {
    D_800D6D54 = stream;
    D_800D6D58 = (u8 *) *stream;
    D_800D6D5C = 0x80;
}

/*
 * PROVENANCE: adapted from JFG's func_80076044_76C44. The bitstream globals
 * are Mickey's, and the compiled result is checked against Mickey's ROM.
 */
s32 func_80050024(u32 bitCount) {
    s32 value;

    value = 0;
    if (bitCount != 0) {
        bitCount = 1 << (bitCount + 0x1F);
        do {
            if (D_800D6D5C == 0) {
                D_800D6D58++;
                D_800D6D5C = 0x80;
            }
            if (*D_800D6D58 & D_800D6D5C) {
                value |= bitCount;
            }
            bitCount >>= 1;
            D_800D6D5C >>= 1;
        } while (bitCount != 0);
    }
    return value;
}

/*
 * PROVENANCE: adapted from JFG's func_800760C0_76CC0. Mickey's ROM fixes the
 * signed-extension expression and all generated instruction choices.
 */
s32 func_800500A4(u32 bitCount) {
    u32 signMask;
    s32 value;

    value = 0;
    if (bitCount != 0) {
        signMask = 0xFFFFFFFF << (bitCount - 1);
        bitCount = 1 << (bitCount - 1);
        do {
            if (D_800D6D5C == 0) {
                D_800D6D58++;
                D_800D6D5C = 0x80;
            }
            if (*D_800D6D58 & D_800D6D5C) {
                value |= bitCount;
            }
            bitCount >>= 1;
            D_800D6D5C >>= 1;
        } while (bitCount != 0);
        if (value & signMask) {
            value |= signMask;
        }
    }
    return value;
}

void func_8005013C(void) {
    if (D_800D6D5C != 0x80) {
        D_800D6D58++;
    }
    *D_800D6D54 = (s32) D_800D6D58;
}


/*
 * Clear the current animation-sequence cursors. The exact JFG donor assembly
 * corroborates the three-global shape; this C is reconstructed from Mickey.
 */
void func_8005017C(void) {
    if (D_8007D698 != NULL) {
        D_8007D698 = NULL;
        D_8007D69C = NULL;
        D_8007D6A0 = NULL;
    }
}

s8 func_800501AC(u16 *entry) {
    return D_8007D6C0[(entry[1] >> 8) & 0xFF];
}

#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_800501C8.s")

/* Exact JFG donor assembly corroborates this setup shape; C is Mickey-led. */
void func_8005027C(void) {
    s32 *base;
    s32 header;

    base = D_8007D68C;
    header = *base;
    D_8007D698 = (u8 *) base + (header & 0xFFFFFF);
    D_8007D69C = D_8007D698;
    D_8007D6A0 = func_800501C8(&D_8007D698);
}

#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_800502CC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80050348.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_8005055C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80050688.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80050704.s")
u32 func_8005077C(u8 pathIndex) {
    AnimPath *path;
    u32 result;

    path = D_800D6B00[pathIndex];
    result = 1;
    if (path != NULL) {
        return (path->flags & 1) == 0;
    }
    return result;
}

#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_800507BC.s")
void animseqLockPath(u8 pathIndex) {
    AnimPath *path;

    path = D_800D6B00[pathIndex];
    if (path != NULL) {
        path->flags |= 8;
    }
}

void animseqUnLockPath(u8 pathIndex) {
    AnimPath *path;

    path = D_800D6B00[pathIndex];
    if (path != NULL) {
        path->flags &= ~8;
    }
}

AnimPath *func_800508B4(u8 pathIndex) {
    return D_800D6B00[pathIndex];
}

#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_800508D4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80050AD4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80050BF4.s")
void func_80050D50(void) {
    void **entry = D_800D6B18, **end = D_800D6B58;
    do {
        if (*entry != NULL) {
            func_80000F74(*entry);
            *entry = NULL;
        }
        entry++;
    } while (entry != end);
}

void animseqFreeLevelData(void) {
    if (D_8007D680 != NULL) {
        mmFree(D_8007D680);
        D_8007D680 = NULL;
        D_8007D688 = -1;
        func_80050E9C();
    }
}

#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80050DF0.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80050E9C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80051004.s")
/* Exact JFG donor assembly corroborates the loop; C is Mickey-led. */
void animseqInitGroup(void) {
    s32 pathIndex;

    pathIndex = 0;
    do {
        func_80050348(pathIndex & 0xFF);
        pathIndex++;
    } while (pathIndex != 0x100);
}

#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80051128.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_800511C4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80051364.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_800517E0.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80053420.s")
/* JFG's ordered anim.c tail and this store establish the tier-D Play name. */
void animseqPlay(void) {
    D_8007D6A4 = 1;
}

void func_800534C0(s32 i) {
    AnimPauseSlot *slot;

    /* The incoming scratch value is replaced before its first use. */
    slot = D_800D6D18;
    i = 4;
    do {
        slot->unkB = 0;
        slot->unk0 = 0;
        slot++;
    } while (i--);
}

#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_800534EC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80053550.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80053868.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80054B3C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80055104.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_800557F8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80055970.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80055B24.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80055D08.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80055E50.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80055F64.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_800560D0.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80056274.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_800563B4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80056DD8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_8005716C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_800572AC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80057350.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_800573C8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_8005776C.s")
/*
 * PROVENANCE: adapted from JFG's src/fmvInit.c. Mickey's ROM establishes the
 * resource ID, globals, structure layout, and final compiler output here.
 */
void fmvInit(void) {
    FmvPlayer *player;
    s32 i;

    D_800D76D0[0] = func_8002E148(0x41);
    player = D_800D76D8;

    i = 2;
    while (i--) {
        player->unk0 = -1;
        player->unk14 = 0;
        player->unk18 = 0;
        player->unk1C = 0;
        player->unk20 = 0;
        player++;
    }
}
