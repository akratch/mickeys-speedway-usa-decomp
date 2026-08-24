#ifndef _N_LIBAUDIO_H_
#define _N_LIBAUDIO_H_

/*
 * Minimal ABI declarations for the one N64 audio-library entry point currently
 * built from C.  The 0x48 event-queue offset is fixed by n_alCSPSetVol's ROM
 * code; keep the opaque player prefix narrow until more audio TUs are ported.
 */

#include "PR/ultratypes.h"

#define AL_SEQP_VOL_EVT 10

typedef s32 ALMicroTime;

typedef struct ALEventQueue_s {
    u8 opaque[20];
} ALEventQueue;

typedef struct N_ALEvent_s {
    s16 type;
    union {
        struct {
            s16 vol;
        } spvol;
        u8 opaque[12];
        u32 align;
    } msg;
} N_ALEvent;

typedef struct N_ALCSPlayer_s {
    u8 opaque[0x48];
    ALEventQueue evtq;
} N_ALCSPlayer;

void n_alEvtqPostEvent(ALEventQueue *evtq, N_ALEvent *evt, ALMicroTime delta);

#endif /* _N_LIBAUDIO_H_ */
