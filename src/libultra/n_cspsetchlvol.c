/*
 * PROVENANCE: adapted from Jet Force Gemini's public decompilation
 * (github.com/Ryan-Myers/Jet-Force-Gemini), a permitted source under
 * docs/CLEANROOM.md; see docs/modules.md section 4.2 (the n_audio
 * synthesis library) and docs/acceleration-survey.md section 13.3.
 */

#include "n_audio/libaudio.h"
#include "n_libaudio.h"

void n_alCSPSetChlVol(N_ALCSPlayer *seqp, u8 chan, u8 vol)
{
    N_ALEvent       evt;

    evt.type            = AL_SEQP_MIDI_EVT;
    evt.msg.midi.ticks  = 0;
    evt.msg.midi.status = AL_MIDI_ControlChange | chan;
    evt.msg.midi.byte1  = AL_MIDI_VOLUME_CTRL;
    evt.msg.midi.byte2  = vol;

    n_alEvtqPostEvent(&seqp->evtq, &evt, 0);
}
