/*
 * PROVENANCE: adapted from Jet Force Gemini's public decompilation
 * (github.com/Ryan-Myers/Jet-Force-Gemini), a permitted source under
 * docs/CLEANROOM.md; see docs/modules.md section 4.2 (the n_audio
 * synthesis library) and docs/reference-findings.md section 3.
 */

#include "n_audio/n_synthInternals.h"

N_ALGlobals *n_alGlobals = 0;
N_ALSynth *n_syn = 0;

void n_alInit(N_ALGlobals *g, ALSynConfig *c)
{
    if (!n_alGlobals) { /* already initialized? */
        n_alGlobals = g;
        if (!n_syn) {
          n_syn = &n_alGlobals->drvr;
          n_alSynNew(c);
        }
    }
}

void n_alClose(N_ALGlobals *glob)
{
    if (n_alGlobals) {
        n_alSynDelete();
        n_alGlobals = 0;
        n_syn = 0;
    }
}
