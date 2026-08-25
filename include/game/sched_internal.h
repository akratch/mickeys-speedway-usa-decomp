#ifndef _GAME_SCHED_INTERNAL_H_
#define _GAME_SCHED_INTERNAL_H_

#include "PR/os_message.h"
#include "PR/sptask.h"

/*
 * PROVENANCE -- field names and the scheduler-prefix layout are adapted from
 * Jet Force Gemini's public decompilation, include/PR/sched.h. Mickey's two
 * byte-identical queue accessors independently prove interruptQ at 0x40 and
 * cmdQ at 0x78. Later fields support the preserved NON_MATCHING retrace draft;
 * they are not claimed as independently matched layout evidence.
 */
typedef struct OSScTask OSScTask;
typedef struct OSScClient OSScClient;

struct OSScTask {
    OSScTask *next;
    u32 state;
    u32 flags;
    void *framebuffer;
    OSTask list;
    OSMesgQueue *msgQ;
    OSMesg msg;
    s32 unk58;
    s32 unk5C;
    s32 unk60;
    s32 unk64;
    s32 unk68;
    s32 taskID;
};

struct OSScClient {
    u8 id;
    u8 pad01[3];
    OSScClient *next;
    OSMesgQueue *msgQ;
};

typedef struct OSSched {
    u8 retraceMsg[0x20];
    u8 prenmiMsg[0x20];
    OSMesgQueue interruptQ;
    OSMesg intBuf[8];
    OSMesgQueue cmdQ;
    OSMesg cmdMsgBuf[8];
    u8 threadAndPad[0x230];
    OSScClient *clientList;
    OSScTask *audioListHead;
    OSScTask *gfxListHead;
    OSScTask *audioListTail;
    OSScTask *gfxListTail;
    OSScTask *curRSPTask;
    OSScTask *curRDPTask;
    OSScTask *unkTask;
    u32 frameCount;
    s32 doAudio;
} OSSched;

#endif /* _GAME_SCHED_INTERNAL_H_ */
