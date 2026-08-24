/*
 * Video-interface and framebuffer management -- ROM 0x34180-0x34E60
 * (VRAM 0x80033580-0x80034260).
 *
 * The translation-unit boundary follows Jet Force Gemini's public gameVi.c:
 * Mickey has the same complete, ordered 23-function sequence from viInit to
 * fb_memcpy. The preceding function is the separately measured
 * trapDanglingJump TU; the next function has the structure and call surface of
 * texInitTextures, the first function in JFG's following textures.c TU.
 *
 * PROVENANCE -- the translation-unit name and function identifications used
 * while reconstructing this file come from JFG's public decompilation,
 * src/gameVi.c and src/gameVi.h. Mickey's own bytes decide every boundary,
 * type, body and matching verdict. Unmatched functions retain their Mickey
 * func_ names below; only evidence-backed names are adopted.
 *
 * Flags: -O2 -mips2 -32, the resident game-code group.
 */

#include "PR/ultratypes.h"

#pragma GLOBAL_ASM("asm/nonmatchings/main/gameVi/func_80033580.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gameVi/func_800336A8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gameVi/func_800339B4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gameVi/func_80033A7C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gameVi/func_80033AD4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gameVi/func_80033B24.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gameVi/func_80033CBC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gameVi/func_80033D04.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gameVi/func_80033D58.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gameVi/func_80033D74.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gameVi/func_80033DA0.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gameVi/func_80033F48.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gameVi/func_80033F5C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gameVi/viSetWideAdjust.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gameVi/func_80033FA8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gameVi/func_80033FB8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gameVi/func_80033FC4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gameVi/func_80033FE0.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gameVi/viDisplayingScreen0.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gameVi/func_80034018.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gameVi/func_80034094.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gameVi/func_80034150.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gameVi/fb_memcpy.s")
