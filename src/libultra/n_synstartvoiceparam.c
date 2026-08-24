/*
 * PROVENANCE: adapted from Jet Force Gemini's public decompilation
 * (github.com/Ryan-Myers/Jet-Force-Gemini), a permitted source under
 * docs/CLEANROOM.md; see docs/modules.md section 4.2 (the n_audio
 * synthesis library) and docs/acceleration-survey.md section 13.3.
 */

#include "n_audio/n_synthInternals.h"
#include <assert.h>

void n_alSynStartVoiceParams( N_ALVoice *v, ALWaveTable *w,
                           f32 pitch, s16 vol, ALPan pan, u8 fxmix,
                           u8 arg6, f32 arg7, u8 arg8,
                           ALMicroTime t)
{
    ALStartParamAlt  *update;

    if (v->pvoice) {
        /*
         * get new update struct from the free list
         */
        update = (ALStartParamAlt *)__n_allocParam();
        ALFailIf(update == 0, ERR_ALSYN_NO_UPDATE);

        /*
         * set offset and fxmix data
         */
#ifdef SAMPLE_ROUND
	update->delta  = SAMPLE184( n_syn->paramSamples + v->pvoice->offset);
#else
        update->delta  = n_syn->paramSamples + v->pvoice->offset;
#endif
        update->next   = 0;
        update->type   = AL_FILTER_START_VOICE_ALT;

        update->unity  = v->unityPitch;
        update->pan    = pan;
        update->volume = vol;
        update->fxMix  = fxmix;
        update->pitch  = pitch;
		update->unk14   = arg8;
		update->unk15   = arg6;
		update->unk18   = arg7;
#ifdef SAMPLE_ROUND
	update->samples = SAMPLE184( _n_timeToSamples( t) );
#else
        update->samples = _n_timeToSamples( t);
#endif
        update->wave    = w;

	n_alEnvmixerParam(v->pvoice, AL_FILTER_ADD_UPDATE, update);
    }

}
