/*
 * PROVENANCE: adapted from Jet Force Gemini's public decompilation
 * (github.com/Ryan-Myers/Jet-Force-Gemini), a permitted source under
 * docs/CLEANROOM.md; see docs/modules.md section 4.2 (the n_audio
 * synthesis library) and docs/acceleration-survey.md section 13.3.
 */

#include "PR/os_message.h"
#include "n_audio/libaudio.h"
#include "n_libaudio.h"

void n_alCSPSetMessageQ(N_ALCSPlayer *seqp, OSMesgQueue *mq) {
    seqp->queue = mq;
}

void n_alCSPSetMessage(N_ALCSPlayer *seqp, u8 index, u8 flag) {
    ALInstrument *inst;

    if (index < seqp->bank->instCount) {
        inst = seqp->bank->instArray[index];
        if (inst != NULL) {
            inst->flags = (flag << 1) | (inst->flags & 1);
        }
    }
}

void n_alCSPSetChanMessage(N_ALCSPlayer *seqp, s32 index, u8 flag) {
    seqp->chanState[index].notemesgflags = flag;
}
