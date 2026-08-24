/*
 * PROVENANCE: adapted from Jet Force Gemini's public decompilation
 * (github.com/Ryan-Myers/Jet-Force-Gemini), a permitted source under
 * docs/CLEANROOM.md; see docs/modules.md section 4.2 (the n_audio
 * synthesis library) and docs/acceleration-survey.md section 13.3.
 */

#include "n_audio/n_synthInternals.h"

void n_alSynStopVoice(N_ALVoice *v)
{
    ALParam  *update;
    ALFilter *f;

    if (v->pvoice) {

        update = __n_allocParam();
        ALFailIf(update == 0, ERR_ALSYN_NO_UPDATE);

        update->delta  = n_syn->paramSamples + v->pvoice->offset;
        update->type   = AL_FILTER_STOP_VOICE;
        update->next   = 0;

        n_alEnvmixerParam(v->pvoice, AL_FILTER_ADD_UPDATE, update);
    }
}
