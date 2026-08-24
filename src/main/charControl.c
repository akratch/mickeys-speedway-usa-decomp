/*
 * Character and camera control -- ROM 0x1C790-0x20020
 * (VRAM 0x8001BB90-0x8001F420).
 *
 * The yaml boundaries were originally splat's aligned file-boundary
 * candidates. The content now supports the TU assignment independently: the
 * first routines follow JFG's camera-control cluster, exact skeleton anchors
 * identify func_8001C2D4 and controlSetPlayerSetup inside the block, the tail
 * is the same player-setup set/get/clear sequence, and the next yaml block
 * starts with a tier-A JFG models.c function. See docs/modules.md section 3.4.
 *
 * PROVENANCE -- Jet Force Gemini's public decomp src/charControl.c,
 * src/charControl.h, built charControl.c object, public symbol map, and
 * asm/nonmatchings/charControl filenames were consulted to identify the
 * translation unit and obtain comparison leads. Names not already supported
 * by tier-A evidence remain comments in docs/modules.md and are not adopted
 * here. Any future body adapted from JFG must carry its own PROVENANCE note
 * before that body and must be proved against Mickey's bytes.
 *
 * Flags: -O2 -mips2 -32, from the measured src/main/ rule.
 */

#include "PR/ultratypes.h"

#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/func_8001BB90.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/func_8001BBB4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/func_8001BE0C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/func_8001C054.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/func_8001C088.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/func_8001C114.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/func_8001C2C4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/func_8001C2D4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/func_8001C320.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/func_8001C4C0.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/func_8001CB0C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/func_8001CB84.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/func_8001D2A0.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/func_8001D41C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/func_8001D638.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/func_8001D690.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/func_8001D824.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/func_8001D880.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/func_8001D910.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/func_8001D960.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/func_8001DCD0.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/func_8001DD70.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/func_8001E5C4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/func_8001EC44.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/func_8001EFFC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/func_8001F09C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/func_8001F14C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/func_8001F25C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/func_8001F264.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/func_8001F320.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/func_8001F364.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/controlSetPlayerSetup.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/func_8001F3AC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/func_8001F408.s")
