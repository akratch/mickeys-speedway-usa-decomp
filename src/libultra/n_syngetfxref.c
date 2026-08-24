/*
 * PROVENANCE: adapted from Jet Force Gemini's public decompilation
 * (github.com/Ryan-Myers/Jet-Force-Gemini), a permitted source under
 * docs/CLEANROOM.md; see docs/modules.md section 4.2 (the n_audio
 * synthesis library) and docs/acceleration-survey.md section 13.3.
 */

#include "n_audio/n_synthInternals.h"

ALFxRef n_alSynGetFXRef(s16 bus)
{
    N_ALMainBus     *m = n_syn->mainBus;

    if (m->filter.handler == (N_ALCmdHandler)n_alFxPull)
        return (ALFxRef)(n_syn->auxBus[bus].fx);
    else
        return 0;
}

ALFxRef n_alSynGetOutputLPRef(s16 bus)
{
    N_ALMainBus     *m = n_syn->mainBus;

    if (m->filter.handler == (void *)n_alFxPull)
    return (ALFxRef)(&n_syn->auxBus[bus].unk44->fx);
    else
    return 0;
}
