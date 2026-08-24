/*
 * PROVENANCE: adapted from Jet Force Gemini's public decompilation
 * (github.com/Ryan-Myers/Jet-Force-Gemini), a permitted source under
 * docs/CLEANROOM.md; see docs/modules.md section 4.2 (the n_audio
 * synthesis library) and docs/acceleration-survey.md section 13.3.
 */

#include "n_audio/n_synthInternals.h"

void n_alSynSetDistort(N_ALVoice *voice, u8 data) {
    ALParam *update;

    if (voice->pvoice != NULL) {
        update = __n_allocParam();
        if (update == NULL) {
            return;
        }
        update->delta = n_syn->paramSamples + voice->pvoice->offset;
        update->type = AL_FILTER_11;
        update->data.i = data;
        update->next = NULL;
        n_alEnvmixerParam(voice->pvoice, AL_FILTER_ADD_UPDATE, update);
    }
}
