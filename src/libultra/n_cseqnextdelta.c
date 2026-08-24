/*
 * PROVENANCE: adapted from Jet Force Gemini's public decompilation
 * (github.com/Ryan-Myers/Jet-Force-Gemini), a permitted source under
 * docs/CLEANROOM.md; see docs/modules.md section 4.2 (the n_audio
 * synthesis library) and docs/acceleration-survey.md section 13.3.
 * JFG folded this routine into n_cseq.c; Mickey's ROM keeps it as its
 * own translation unit (ROM 0x66970, 0xE0 bytes), so it stays split out
 * here to match the measured file boundary.
 */

#include "n_audio/libaudio.h"

s32 __alCSeqNextDelta(ALCSeq *seq, s32 *pDeltaTicks) {
    u32 i;
    u32 firstTime = 0xffffffff;
    u32 lastTicks = seq->lastDeltaTicks;

    if (!seq->validTracks) {
        return FALSE;
    }

    for (i = 0; i < 16; i++) {
        if ((seq->validTracks >> i) & 1) {
            if (seq->deltaFlag) {
                seq->evtDeltaTicks[i] -= lastTicks;
            }

            if (seq->evtDeltaTicks[i] < firstTime) {
                firstTime = seq->evtDeltaTicks[i];
            }
        }
    }

    seq->deltaFlag = 0;
    *pDeltaTicks = firstTime;

    return TRUE;
}
