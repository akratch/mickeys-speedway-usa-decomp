/*
 * Resident positional-audio manager -- ROM 0x3100-0x45F0
 * (VRAM 0x80002500).
 *
 * PROVENANCE: the translation-unit identity and candidate routine names were
 * compared with Jet Force Gemini's public decomp, src/audio_manager_36D0.c,
 * which is a permitted source under docs/CLEANROOM.md. Adapted bodies carry
 * their own point-of-use disclosure. Mickey's boundaries remain authoritative.
 *
 * func_80002500 has JFG amInitAudioMap's initialization role and starts at a
 * 16-byte boundary. The following twenty functions preserve JFG's TU order;
 * the last ends exactly where the byte-identified amVibratoInit begins.
 *
 * Flags: -O2 -mips2 -32, from the measured src/main/ flag group.
 */

#include "PR/ultratypes.h"

typedef struct AudioPoint {
    f32 x;
    f32 y;
    f32 z;
    u16 soundId;
    u8 volume;
    u8 pitch;
} AudioPoint;

#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_36D0/func_80002500.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_36D0/audspat_jingle_off.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_36D0/func_800025EC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_36D0/func_800025F8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_36D0/func_80002768.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_36D0/func_80002E88.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_36D0/func_80002FE0.s")
/* PROVENANCE: body adapted from JFG src/audio_manager_36D0.c amSndSetVolXYZ. */
void func_8000309C(AudioPoint *point, u8 volume) {
    if (point != NULL) {
        point->volume = volume;
    }
}

/* PROVENANCE: body adapted from JFG src/audio_manager_36D0.c amSndSetPitchXYZ. */
void func_800030B4(AudioPoint *point, u8 pitch) {
    if (point != NULL) {
        point->pitch = pitch;
    }
}

#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_36D0/func_800030CC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_36D0/func_800031C0.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_36D0/func_800031E8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_36D0/func_80003250.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_36D0/func_8000329C.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_36D0/func_800033B0.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_36D0/func_80003480.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_36D0/func_800035F8.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_36D0/func_80003760.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_36D0/func_800037C4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_36D0/func_800038EC.s")
