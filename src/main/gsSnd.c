/*
 * Rare sound player -- ROM 0x5C310-0x5E6B0
 * (VRAM 0x8005B710-0x8005DAB0).
 *
 * Tier A: the complete 0x23A0-byte text is byte-identical, under relocation
 * masking, to JFG's built src/gsSnd.c.o. The first split keeps every function
 * in generated assembly; matching bodies are promoted one at a time. A flag
 * lattice on gsSndpGetGlobalVolume found the TU's debug-shaped duplicate
 * epilogues exact only under bare -g (with -mips2 -32), not the game default
 * -O2, so the Makefile carries that measured per-file override.
 *
 * PROVENANCE: JFG's permitted src/gsSnd.c and src/gsSnd.h were read for the
 * function names, declarations and source candidates. Adapted bodies will be
 * identified at their point of use and remain subject to Mickey byte identity.
 */

#include "PR/ultratypes.h"

extern u32 D_8007FF50;
extern u16 *D_800D7D78;

typedef struct GsSndPriorityState {
    u8 pad0[0x40];
    u8 priority;
    u8 pad41[3];
    u8 state;
} GsSndPriorityState;

#pragma GLOBAL_ASM("asm/nonmatchings/main/gsSnd/gsSndpNew.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gsSnd/func_8005B978.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gsSnd/func_8005BA40.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gsSnd/func_8005CD3C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gsSnd/func_8005CDAC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gsSnd/func_8005CE28.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gsSnd/getSoundStateCounts.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gsSnd/func_8005D030.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gsSnd/func_8005D260.s")
/* PROVENANCE: adapted from JFG src/gsSnd.c (gsSndpSetPriority). */
void gsSndpSetPriority(GsSndPriorityState *state, u8 priority) {
    if (state != NULL) {
        state->priority = (s16)priority;
    }
}
/* PROVENANCE: adapted from JFG src/gsSnd.c (gsSndpGetState). */
u8 gsSndpGetState(GsSndPriorityState *state) {
    if (state != NULL) {
        return state->state;
    } else {
        return 0;
    }
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/gsSnd/ad_sndp_play.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gsSnd/gsSndpStop.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gsSnd/sndp_stop_with_flags.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gsSnd/gsSndpStopAll.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gsSnd/gsSndpStopAllRetrigger.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gsSnd/gsSndpStopAllLooped.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gsSnd/gsSndpSetParam.s")
/* PROVENANCE: adapted from JFG src/gsSnd.c (gsSndpGetMasterVolume). */
u16 gsSndpGetMasterVolume(u8 groupID) {
    return D_800D7D78[groupID];
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/gsSnd/gsSndpSetMasterVolume.s")
/* PROVENANCE: adapted from JFG src/gsSnd.c (gsSndpSetGlobalVolume). */
void gsSndpSetGlobalVolume(u32 volume) {
    if (volume > 0x100) {
        volume = 0x100;
    }
    D_8007FF50 = volume;
}
/* PROVENANCE: adapted from JFG src/gsSnd.c (gsSndpGetGlobalVolume). */
u32 gsSndpGetGlobalVolume(void) {
    return D_8007FF50;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/gsSnd/gsSndpLimitVoices.s")
