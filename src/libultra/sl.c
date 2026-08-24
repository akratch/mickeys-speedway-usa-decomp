/*
 * PROVENANCE: adapted from Jet Force Gemini's public decompilation
 * (github.com/Ryan-Myers/Jet-Force-Gemini), a permitted source under
 * docs/CLEANROOM.md; see docs/modules.md section 4.2 (the n_audio
 * synthesis library) and docs/acceleration-survey.md section 13.3.
 */

#include "n_audio/libaudio.h"

/* might want to make these macros */
void alLink(ALLink *ln, ALLink *to)
{
    ln->next = to->next;
    ln->prev = to;
    if (to->next)
        to->next->prev = ln;
    to->next = ln;
}

void alUnlink(ALLink *ln)
{
    if (ln->next)
        ln->next->prev = ln->prev;
    if (ln->prev)
        ln->prev->next = ln->next;
}
