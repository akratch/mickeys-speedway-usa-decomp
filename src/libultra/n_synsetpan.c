/*
 * PROVENANCE: adapted from Jet Force Gemini's public decompilation
 * (github.com/Ryan-Myers/Jet-Force-Gemini), a permitted source under
 * docs/CLEANROOM.md; see docs/modules.md section 4.2 (the n_audio
 * synthesis library) and docs/acceleration-survey.md section 13.3.
 */

#include "n_audio/n_synthInternals.h"

void n_alSynSetPan(N_ALVoice *v, u8 pan)
{
    ALParam  *update;

    if (v->pvoice) {

        /*
         * get new update struct from the free list
         */
        update = __n_allocParam();
        ALFailIf(update == 0, ERR_ALSYN_NO_UPDATE);

        /*
         * set offset and pan data
         */
        update->delta  = n_syn->paramSamples + v->pvoice->offset;
        update->type   = AL_FILTER_SET_PAN;
        update->data.i = pan;
        update->next   = 0;

        n_alEnvmixerParam(v->pvoice, AL_FILTER_ADD_UPDATE, update);
    }
}
