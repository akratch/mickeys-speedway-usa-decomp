#ifndef _OS_MESSAGE_H_
#define _OS_MESSAGE_H_

/*
 * libultra's message queues.
 *
 * The field order and offsets are an ABI fact about the ROM, not a choice:
 * `__osPiAccessQueue` at 0x800D84E0 is 0x18 bytes wide (the next object,
 * `piAccessBuf`, starts at 0x800D84F8), and osCreateMesgQueue is called with
 * (queue, buffer, 1) at ROM 0x72754.
 *
 * PROVENANCE: names and layout follow the N64 SDK header as published in
 * public decomp trees (JFG's `include/PR/os_message.h`), a permitted source
 * under docs/CLEANROOM.md; see docs/modules.md section 1.3.
 */

#include "PR/ultratypes.h"
#include "PR/os_internal.h"

typedef void *OSMesg;

typedef struct OSMesgQueue_s {
    OSThread *mtqueue;   /* threads blocked receiving on an empty mailbox */
    OSThread *fullqueue; /* threads blocked sending on a full mailbox */
    s32 validCount;      /* messages currently held */
    s32 first;           /* index of the oldest valid message */
    s32 msgCount;        /* capacity of `msg` */
    OSMesg *msg;         /* the caller's message buffer */
} OSMesgQueue;

#define OS_MESG_NOBLOCK 0
#define OS_MESG_BLOCK   1

void osCreateMesgQueue(OSMesgQueue *mq, OSMesg *msg, s32 count);
s32 osSendMesg(OSMesgQueue *mq, OSMesg msg, s32 flag);
s32 osJamMesg(OSMesgQueue *mq, OSMesg msg, s32 flag);
s32 osRecvMesg(OSMesgQueue *mq, OSMesg *msg, s32 flag);

#endif /* _OS_MESSAGE_H_ */
