/*
 * PROVENANCE: N64 SDK libultra source as published in the permitted Jet
 * Force Gemini decompilation. Its built whole-TU object is byte-identical
 * to Mickey; see symbol_addrs.us.txt and docs/modules.md section 1.3.
 */

#include "PR/os_internal.h"
#include "PR/ultraerror.h"
#include "PRinternal/osint.h"

void osCreateMesgQueue(OSMesgQueue* mq, OSMesg* msg, s32 msgCount) {

#ifdef _DEBUG
    if (msgCount <= 0) {
        __osError(ERR_OSCREATEMESGQUEUE, 1, msgCount);
        return;
    }
#endif

    mq->mtqueue = (OSThread*)&__osThreadTail.next;
    mq->fullqueue = (OSThread*)&__osThreadTail.next;
    mq->validCount = 0;
    mq->first = 0;
    mq->msgCount = msgCount;
    mq->msg = msg;
}
