/*
 * PROVENANCE: adapted from Jet Force Gemini's public decompilation
 * (github.com/Ryan-Myers/Jet-Force-Gemini), a permitted source under
 * docs/CLEANROOM.md; see docs/modules.md section 4.2 (the n_audio
 * synthesis library) and docs/reference-findings.md section 3.
 */

#include "n_audio/n_synthInternals.h"

void n_alSynFreeVoice(N_ALVoice *voice)
{
    ALFilter *f;
    N_ALFreeParam *update;

    if (voice->pvoice) {
        if (voice->pvoice->offset) {
            update = (N_ALFreeParam *)__n_allocParam();
            ALFailIf(update == 0, ERR_ALSYN_NO_UPDATE);

            update->delta = n_syn->paramSamples + voice->pvoice->offset;
            update->type = AL_FILTER_FREE_VOICE;
            update->pvoice = voice->pvoice;

            n_alEnvmixerParam(voice->pvoice, AL_FILTER_ADD_UPDATE, update);
        } else {
            _n_freePVoice(voice->pvoice);
        }
        voice->pvoice = 0;
    }
}
