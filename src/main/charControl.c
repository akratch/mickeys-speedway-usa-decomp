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
#include "game/charControl.h"

extern u8 D_80079BF8;
extern s16 D_800CB470;
extern s16 D_800CB472;
extern s16 D_800CB474;
extern s16 D_800CB476;

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
/* PROVENANCE -- adapted from JFG's src/charControl.c dAngle. */
s16 dAngle(s16 arg0, s16 arg1, f32 arg2) {
    s32 temp_t1;
    s32 var_v1;

    var_v1 = (arg1 - arg0) & 0xFFFF;
    temp_t1 = (arg0 - arg1) & 0xFFFF;
    if (temp_t1 < var_v1) {
        var_v1 = -temp_t1;
    }
    return (s16) (arg0 + (s32) ((f32) var_v1 * arg2));
}
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
/*
 * PROVENANCE -- JFG's src/charControl.c supplied the controlDisableJoypad
 * name/role. Mickey's two-argument field store independently determines this
 * per-player body and differs from JFG's one-argument global implementation.
 */
void controlDisableJoypad(ControlPlayer *player, s32 disabled) {
    player->joypadDisabled = disabled;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/func_8001F264.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/func_8001F320.s")
void func_8001F364(void) {
}
void controlSetPlayerSetup(s16 arg0, s16 arg1, s16 arg2, s16 arg3) {
    D_800CB470 = arg0;
    D_800CB472 = arg1;
    D_800CB474 = arg2;
    D_800CB476 = arg3;
    D_80079BF8 = 1;
}
/*
 * PROVENANCE -- JFG's charControl symbols supplied the controlGetPlayerSetup
 * name/role. This body is reconstructed from Mickey's setup-state accesses.
 */
s32 controlGetPlayerSetup(s16 *arg0, s16 *arg1, s16 *arg2, s16 *arg3) {
    if (D_80079BF8 != 0) {
        *arg0 = D_800CB470;
        *arg1 = D_800CB472;
        *arg2 = D_800CB474;
        *arg3 = D_800CB476;
        D_80079BF8 = 0;
        return 1;
    }
    return 0;
}

/* PROVENANCE -- adapted from JFG's src/charControl.c controlClearPlayerSetup. */
void controlClearPlayerSetup(void) {
    D_80079BF8 = 0;
}
