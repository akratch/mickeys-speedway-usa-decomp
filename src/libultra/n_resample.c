/*
 * PROVENANCE: adapted from Jet Force Gemini's public decompilation
 * (github.com/Ryan-Myers/Jet-Force-Gemini), a permitted source under
 * docs/CLEANROOM.md; see docs/modules.md section 4.4 and
 * docs/acceleration-survey.md section 13.3.
<<<<<<< HEAD
 * JFG source: libultra/src/naudio/n_resample.c at c75c270.
 */

#include "n_audio/n_synthInternals.h"
=======
 */

/*====================================================================
 *
 * Copyright 1993, Silicon Graphics, Inc.
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Silicon Graphics,
 * Inc.; the contents of this file may not be disclosed to third
 * parties, copied or duplicated in any form, in whole or in part,
 * without the prior written permission of Silicon Graphics, Inc.
 *
 * RESTRICTED RIGHTS LEGEND:
 * Use, duplication or disclosure by the Government is subject to
 * restrictions as set forth in subdivision (c)(1)(ii) of the Rights
 * in Technical Data and Computer Software clause at DFARS
 * 252.227-7013, and/or in similar or successor clauses in the FAR,
 * DOD or NASA FAR Supplement. Unpublished - rights reserved under the
 * Copyright Laws of the United States.
 *====================================================================*/

#include "n_audio/n_synthInternals.h"

>>>>>>> lane/cx-naudio-a
#include <os.h>

#ifdef AUD_PROFILE
extern u32 cnt_index, resampler_num, resampler_cnt, resampler_max, resampler_min, lastCnt[];
#endif

<<<<<<< HEAD
=======
/***********************************************************************
 * Resampler filter public interfaces
 ***********************************************************************/
>>>>>>> lane/cx-naudio-a
Acmd *n_alResamplePull(N_PVoice *e, s16 *outp, Acmd *p)
{
    Acmd *ptr = p;
    s16 inp;
    s32 inCount;
    s32 incr;
    f32 finCount;

#ifdef AUD_PROFILE
    lastCnt[++cnt_index] = osGetCount();
#endif

#ifndef N_MICRO
    inp = AL_DECODER_OUT;
#else
    inp = N_AL_DECODER_OUT;
#endif

<<<<<<< HEAD
=======
    /*
     * check if resampler is required
     */
>>>>>>> lane/cx-naudio-a
    if (e->rs_upitch) {
        ptr = n_alAdpcmPull(e, &inp, FIXED_SAMPLE, p);
        aDMEMMove(ptr++, inp, *outp, FIXED_SAMPLE << 1);
    } else {
<<<<<<< HEAD
=======
        /*
         * clip to maximum allowable pitch
         * FIXME: should we check for some minimum as well?
         */
>>>>>>> lane/cx-naudio-a
        if (e->rs_ratio > MAX_RATIO) {
            e->rs_ratio = MAX_RATIO;
        }

<<<<<<< HEAD
        e->rs_ratio = (s32)(e->rs_ratio * UNITY_PITCH);
        e->rs_ratio = e->rs_ratio / UNITY_PITCH;

=======
        /*
         * quantize the pitch
         */
        e->rs_ratio = (s32)(e->rs_ratio * UNITY_PITCH);
        e->rs_ratio = e->rs_ratio / UNITY_PITCH;

        /*
         * determine how many samples to generate
         */
>>>>>>> lane/cx-naudio-a
        finCount = e->rs_delta + (e->rs_ratio * (f32)FIXED_SAMPLE);
        inCount = (s32)finCount;
        e->rs_delta = finCount - (f32)inCount;

<<<<<<< HEAD
        ptr = n_alAdpcmPull(e, &inp, inCount, p);
=======
        /*
         * ask all filters upstream from us to build their command
         * lists.
         */
        ptr = n_alAdpcmPull(e, &inp, inCount, p);

        /*
         * construct our portion of the command list
         */
>>>>>>> lane/cx-naudio-a
        incr = (s32)(e->rs_ratio * UNITY_PITCH);
#ifndef N_MICRO
        aSetBuffer(ptr++, 0, inp, *outp, FIXED_SAMPLE << 1);
        aResample(ptr++, e->rs_first, incr, osVirtualToPhysical(e->rs_state));
#else
        n_aResample(ptr++, osVirtualToPhysical(e->rs_state), e->rs_first, incr, inp, 0);
#endif
        e->rs_first = 0;
    }

#ifdef AUD_PROFILE
    PROFILE_AUD(resampler_num, resampler_cnt, resampler_max, resampler_min);
#endif
    return ptr;
}

s32 n_alResampleParam(N_PVoice *filter, s32 paramID, void *param)
{
    N_PVoice *r = filter;
    union {
        f32 f;
        s32 i;
    } data;

    switch (paramID) {
#if 0
<<<<<<< HEAD
        case AL_FILTER_RESET:
            r->rs_delta = 0.0;
            r->rs_first = 1;
            r->rs_upitch = 0;
            n_alLoadParam(filter, AL_FILTER_RESET, 0);
            break;
        case AL_FILTER_START:
            n_alLoadParam(filter, AL_FILTER_START, 0);
            break;
        case AL_FILTER_SET_PITCH:
            data.i = (s32)param;
            r->rs_ratio = data.f;
            break;
        case AL_FILTER_SET_UNITY_PITCH:
            r->rs_upitch = 1;
            break;
#endif
        default:
            n_alLoadParam(filter, paramID, param);
            break;
=======
    case AL_FILTER_RESET:
        r->rs_delta = 0.0;
        r->rs_first = 1;
        r->rs_upitch = 0;
        n_alLoadParam(filter, AL_FILTER_RESET, 0);
        break;
    case AL_FILTER_START:
        n_alLoadParam(filter, AL_FILTER_START, 0);
        break;
    case AL_FILTER_SET_PITCH:
        data.i = (s32)param;
        r->rs_ratio = data.f;
        break;
    case AL_FILTER_SET_UNITY_PITCH:
        r->rs_upitch = 1;
        break;
#endif
    default:
        n_alLoadParam(filter, paramID, param);
        break;
>>>>>>> lane/cx-naudio-a
    }
    return 0;
}
