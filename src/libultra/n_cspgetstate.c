/*
 * PROVENANCE: adapted from Jet Force Gemini's public decompilation
 * (github.com/Ryan-Myers/Jet-Force-Gemini), a permitted source under
 * docs/CLEANROOM.md; see docs/modules.md section 4.2 (the n_audio
 * synthesis library) and docs/reference-findings.md section 3.
 */

#include "n_audio/libaudio.h"
#include "n_libaudio.h"

s32 n_alCSPGetState(N_ALCSPlayer *seqp)
{
    return seqp->state;
}
