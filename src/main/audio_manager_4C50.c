/*
 * Resident audio oscillators -- ROM 0x45F0-0x4F40 (VRAM 0x800039F0).
 *
 * PROVENANCE: the translation-unit identity and candidate routine names were
 * compared with Jet Force Gemini's public decomp, src/audio_manager_4C50.c,
 * which is a permitted source under docs/CLEANROOM.md. No C body is adapted
 * here yet. Mickey's own boundaries and symbols remain authoritative.
 *
 * Both ends are measured: amVibratoInit and _depth2Cents are tier-A JFG byte
 * identities, all five functions preserve JFG's order, and _depth2Cents ends
 * four padding bytes before the existing 0x4F40 boundary.
 *
 * Flags: -O2 -mips2 -32, from the measured src/main/ flag group.
 */

#include "PR/ultratypes.h"

#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_4C50/amVibratoInit.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_4C50/func_80003A80.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_4C50/func_80003D58.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_4C50/func_800042CC.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_4C50/_depth2Cents.s")
