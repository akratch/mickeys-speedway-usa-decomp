/*
 * PROVENANCE: N64 SDK libultra source as published in the permitted Jet
 * Force Gemini decompilation. Its built whole-TU object is byte-identical
 * to Mickey; see symbol_addrs.us.txt and docs/modules.md section 1.3.
 */

#include "PR/os_internal.h"
#include "PR/ultraerror.h"
#include "assert.h"
#include "PRinternal/viint.h"












































// TODO: this comes from a header
#ident "$Revision: 1.17 $"

void osViSetEvent(OSMesgQueue* mq, OSMesg m, u32 retraceCount) {
    register u32 saveMask;

#ifdef _DEBUG
    if (!__osViDevMgr.active) {
        __osError(ERR_OSVISETEVENT, 0);
        return;
    }

    assert(mq != NULL);
#endif

    saveMask = __osDisableInt();

    __osViNext->msgq = mq;
    __osViNext->msg = m;
    __osViNext->retraceCount = retraceCount;
    __osRestoreInt(saveMask);
}
