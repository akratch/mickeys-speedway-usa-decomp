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

#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80050000.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80050024.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_800500A4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_8005013C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_8005017C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_800501AC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_800501C8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_8005027C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_800502CC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80050348.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_8005055C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80050688.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80050704.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_8005077C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_800507BC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80050844.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_8005087C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_800508B4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_800508D4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80050AD4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80050BF4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80050D50.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80050DA8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80050DF0.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80050E9C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80051004.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/animseqInitGroup.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80051128.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_800511C4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80051364.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_800517E0.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80053420.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_800534B0.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_800534C0.s")
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
#pragma GLOBAL_ASM("asm/nonmatchings/main/anim/func_80057910.s")
