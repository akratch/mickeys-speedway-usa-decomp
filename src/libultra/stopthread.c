/*
 * PROVENANCE: N64 SDK libultra source as published in the permitted Jet
 * Force Gemini decompilation. Its built whole-TU object is byte-identical
 * to Mickey; see symbol_addrs.us.txt and docs/modules.md section 1.3.
 */

#include "PR/os_internal.h"
#include "PRinternal/osint.h"

void osStopThread(OSThread* t) {
    register u32 saveMask = __osDisableInt();
    register u16 state;

    state = (t == NULL) ? OS_STATE_RUNNING: t->state;

    switch (state) {
        case OS_STATE_RUNNING:
            __osRunningThread->state = OS_STATE_STOPPED;
            __osEnqueueAndYield(NULL);
            break;
        case OS_STATE_RUNNABLE:
        case OS_STATE_WAITING:
            t->state = OS_STATE_STOPPED;
            __osDequeueThread(t->queue, t);
            break;
    }

    __osRestoreInt(saveMask);
}
