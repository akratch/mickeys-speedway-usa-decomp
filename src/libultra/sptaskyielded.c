/*
 * PROVENANCE: N64 SDK libultra source as published in the permitted Jet
 * Force Gemini decompilation. Its built whole-TU object is byte-identical
 * to Mickey; see symbol_addrs.us.txt and docs/modules.md section 1.3.
 */

#include "PR/os_internal.h"
#include "PR/sptask.h"
#include "PR/rcp.h"

OSYieldResult osSpTaskYielded(OSTask* tp) {
    u32 status;
    OSYieldResult result;

    status = __osSpGetStatus();
    result = (status & SP_STATUS_YIELDED) ? OS_TASK_YIELDED : 0;

    if (status & SP_STATUS_YIELD) {
        tp->t.flags |= result;
        tp->t.flags &= ~(OS_TASK_DP_WAIT);
    }

    return result;
}
