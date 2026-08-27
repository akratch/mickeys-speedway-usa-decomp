/*
 * PROVENANCE: adapted from Jet Force Gemini's public decompilation
 * (github.com/Ryan-Myers/Jet-Force-Gemini), a permitted source under
 * docs/CLEANROOM.md; see docs/modules.md section 4.2 (the n_audio
 * synthesis library) and docs/reference-findings.md section 3.
 */

#include <os_internal.h>
#include "n_audio/n_synthInternals.h"

#define OS_IM_NONE 0x00000001

OSIntMask osSetIntMask(OSIntMask mask);

void n_alSynAddPlayer( ALPlayer *client)
{
    OSIntMask mask = osSetIntMask(OS_IM_NONE);

    client->samplesLeft = n_syn->curSamples;

    client->next = n_syn->head;
    n_syn->head   = client;

    osSetIntMask(mask);
}


void n_alSynAddSndPlayer( ALPlayer *client)
{
    OSIntMask mask = osSetIntMask(OS_IM_NONE);

    client->samplesLeft = n_syn->curSamples;

#if 1
    client->next = n_syn->head;
    n_syn->head   = client;
#endif

    osSetIntMask(mask);
}

void n_alSynAddSeqPlayer( ALPlayer *client)
{
    OSIntMask mask = osSetIntMask(OS_IM_NONE);

    client->samplesLeft = n_syn->curSamples;

#if 1
    client->next = n_syn->head;
    n_syn->head   = client;
#endif

    osSetIntMask(mask);
}
