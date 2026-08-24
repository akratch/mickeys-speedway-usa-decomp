/*
 * Resident audio front end -- ROM 0x1050-0x2340 (VRAM 0x80000450).
 *
 * PROVENANCE: the translation-unit identity and candidate routine names were
 * compared with Jet Force Gemini's public decomp, src/audio_manager_1050.c,
 * which is a permitted source under docs/CLEANROOM.md. Adapted bodies carry
 * their own point-of-use disclosure. Mickey's boundaries remain authoritative.
 *
 * The end is the 16-byte-aligned start of Mickey's audiomgr-shaped run:
 * func_80001740 has the allocator, message-queue, and audio-thread setup shape
 * of JFG's first audiomgr routine. The two preceding Mickey-only routines stay
 * with this TU; no unaligned boundary is asserted from JFG's layout.
 *
 * Flags: -O2 -mips2 -32, from the measured src/main/ flag group.
 */

#include "PR/ultratypes.h"

extern s32 D_80078D7C;
extern s32 D_80078D80;
extern void gsSndpSetParam(void *sound, s16 type, u32 value);
extern void n_alCSPNew(void *player, void *config);
extern void n_alCSPSetMessageQ(void *player, void *queue);

#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_80000450.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_80000510.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_80000594.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_800005CC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/amTuneSetFadeScaled.s")
/* PROVENANCE: body and name adapted from JFG src/audio_manager_1050.c. */
void amTuneResetFade(void) {
    D_80078D7C = 0;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_8000073C.s")
/* PROVENANCE: body and name adapted from JFG src/audio_manager_1050.c. */
void amAmbientResetFade(void) {
    D_80078D80 = 0;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_80000838.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_80000ABC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_80000B3C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_80000B48.s")
/* PROVENANCE: name/order compared with JFG src/audio_manager_1050.c. */
void amTuneMuteChl(s32 channel) {
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_80000BF0.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_80000BF8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_80000C38.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_80000C9C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_80000CEC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_80000D1C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_80000D54.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_80000D90.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_80000D9C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_80000E08.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_80000E64.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_80000E70.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_80000EBC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_80000F20.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_80000F74.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_80000F94.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_80001098.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_80001144.s")
/* PROVENANCE: name/order compared with JFG src/audio_manager_1050.c. */
void amSndSetPan(void *sound, u32 pan) {
    if (sound != NULL) {
        gsSndpSetParam(sound, 4, pan);
    }
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_8000122C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_80001258.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_80001270.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_800012A8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_80001308.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_8000137C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_80001568.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_800015F0.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_800015F8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_80001608.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_80001614.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_80001620.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_80001668.s")
/* PROVENANCE: name/order compared with JFG src/audio_manager_1050.c. */
void forcelink(void) {
    n_alCSPNew(NULL, NULL);
    n_alCSPSetMessageQ(NULL, NULL);
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_800016C8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_800016EC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_1050/func_80001708.s")
