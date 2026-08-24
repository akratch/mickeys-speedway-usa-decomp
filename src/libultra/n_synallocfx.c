/*
 * PROVENANCE: adapted from Jet Force Gemini's public decompilation
 * (github.com/Ryan-Myers/Jet-Force-Gemini), a permitted source under
 * docs/CLEANROOM.md; see docs/modules.md section 4.2 (the n_audio
 * synthesis library) and docs/acceleration-survey.md section 13.3.
 */

#include "n_audio/n_synthInternals.h"

ALFxRef n_alSynAllocFX(s16 bus, ALSynConfig *c, ALHeap *hp)
{
    n_alFxNew(n_syn->auxBus[bus].fx_array, c, bus, hp);
    return (n_syn->auxBus[bus].fx_array[0]);
}
