/*
 * Rare sound player -- ROM 0x5C310-0x5E6B0
 * (VRAM 0x8005B710-0x8005DAB0).
 *
 * Tier A: the complete 0x23A0-byte text is byte-identical, under relocation
 * masking, to JFG's built src/gsSnd.c.o. The first split keeps every function
 * in generated assembly; matching bodies are promoted one at a time.
 *
 * PROVENANCE: JFG's permitted src/gsSnd.c and src/gsSnd.h were read for the
 * function names, declarations and source candidates. Adapted bodies will be
 * identified at their point of use and remain subject to Mickey byte identity.
 */

#pragma GLOBAL_ASM("asm/nonmatchings/main/gsSnd/gsSndpNew.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gsSnd/func_8005B978.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gsSnd/func_8005BA40.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gsSnd/func_8005CD3C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gsSnd/func_8005CDAC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gsSnd/func_8005CE28.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gsSnd/getSoundStateCounts.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gsSnd/func_8005D030.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gsSnd/func_8005D260.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gsSnd/gsSndpSetPriority.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gsSnd/gsSndpGetState.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gsSnd/ad_sndp_play.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gsSnd/gsSndpStop.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gsSnd/sndp_stop_with_flags.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gsSnd/gsSndpStopAll.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gsSnd/gsSndpStopAllRetrigger.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gsSnd/gsSndpStopAllLooped.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gsSnd/gsSndpSetParam.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gsSnd/gsSndpGetMasterVolume.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gsSnd/gsSndpSetMasterVolume.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gsSnd/gsSndpSetGlobalVolume.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gsSnd/gsSndpGetGlobalVolume.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/gsSnd/gsSndpLimitVoices.s")
