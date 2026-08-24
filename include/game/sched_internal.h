#ifndef _GAME_SCHED_INTERNAL_H_
#define _GAME_SCHED_INTERNAL_H_

#include "PR/os_message.h"

/*
 * PROVENANCE -- field names and the scheduler-prefix layout are adapted from
 * Jet Force Gemini's public decompilation, include/PR/sched.h. Mickey's two
 * byte-identical queue accessors independently prove interruptQ at 0x40 and
 * cmdQ at 0x78. Only the prefix required by those accessors is declared here.
 */
typedef struct OSSched {
    u8 retraceMsg[0x20];
    u8 prenmiMsg[0x20];
    OSMesgQueue interruptQ;
    OSMesg intBuf[8];
    OSMesgQueue cmdQ;
} OSSched;

#endif /* _GAME_SCHED_INTERNAL_H_ */
