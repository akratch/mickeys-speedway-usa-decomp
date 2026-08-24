/*
 * PROVENANCE: adapted from Jet Force Gemini's public decompilation
 * (github.com/Ryan-Myers/Jet-Force-Gemini), a permitted source under
<<<<<<< HEAD
 * docs/CLEANROOM.md; see docs/modules.md section 4.4 and
 * docs/acceleration-survey.md section 13.3.
 * JFG source: libultra/src/naudio/n_synsetfxparam.c at c75c270.
=======
 * docs/CLEANROOM.md; see docs/modules.md section 4.2 and
 * docs/acceleration-survey.md section 13.3.
>>>>>>> lane/cx-naudio-a
 */

#include "n_audio/n_synthInternals.h"

void n_alSynSetFXParam(ALFxRef fx, s16 paramID, void *param)
{
    ALFx *f = (ALFx *)fx;

    n_alFxParamHdl(f, (s32)paramID, param);
}

void n_alFxInitlpfilter_mono(struct fx *fx, f32 outputrate);

void n_alSynSetOutputLPParam(struct fx *fx, s16 arg1, void *param)
{
    if (arg1 == 8) {
        fx->unk02 = (*(s32 *)param * 0.1f);
    } else if (arg1 == 9) {
        fx->unk00 = *(s32 *)param;
    }

    n_alFxInitlpfilter_mono(fx, n_syn->outputRate);
}
