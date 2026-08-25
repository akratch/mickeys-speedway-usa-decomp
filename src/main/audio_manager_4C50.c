/*
 * Resident audio oscillators -- ROM 0x45F0-0x4F40 (VRAM 0x800039F0).
 *
 * PROVENANCE: the translation-unit identity and candidate routine names were
 * compared with Jet Force Gemini's public decomp, src/audio_manager_4C50.c,
 * which is a permitted source under docs/CLEANROOM.md. Adapted bodies carry
 * their own point-of-use disclosure. Mickey's boundaries remain authoritative.
 *
 * Both ends are measured: amVibratoInit and _depth2Cents are tier-A JFG byte
 * identities, all five functions preserve JFG's order, and _depth2Cents owns
 * 0x50 executable bytes followed by 0xC bytes of terminal alignment.
 *
 * Flags: -O2 -mips2 -32 -Wab,-r4300_mul, confirmed by the flag lattice.
 */

#include "PR/ultratypes.h"

typedef struct AudioOscillatorState {
    struct AudioOscillatorState *next;
} AudioOscillatorState;

extern f32 D_80080BA8;
extern AudioOscillatorState *D_800C9300;

#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_4C50/amVibratoInit.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_4C50/func_80003A80.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/audio_manager_4C50/func_80003D58.s")
/*
 * PROVENANCE: name/order compared with JFG src/audio_manager_4C50.c amStopOsc;
 * body uses Mickey-only evidence.
 */
void amStopOsc(AudioOscillatorState *state) {
    state->next = D_800C9300;
    D_800C9300 = state;
}

/* PROVENANCE: body adapted from JFG src/audio_manager_4C50.c and BK src/core1/code_1D00.c. */
f32 _depth2Cents(u8 depth) {
    f32 ratio;
    f32 result;

    ratio = D_80080BA8;
    result = 1.0f;
    while (depth) {
        if (depth & 1) {
            result *= ratio;
        }
        ratio *= ratio;
        depth >>= 1;
    }
    return result;
}
