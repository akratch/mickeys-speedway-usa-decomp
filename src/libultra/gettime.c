/*
 * PROVENANCE: N64 SDK libultra source as published in the permitted Jet
 * Force Gemini decompilation. Its built whole-TU object is byte-identical
 * to Mickey; see symbol_addrs.us.txt and docs/modules.md section 1.3.
 */

#include "PR/os_internal.h"
#include "PR/ultraerror.h"
#include "PRinternal/osint.h"

OSTime osGetTime(void) {
    u32 tmptime;
    u32 elapseCount;
    OSTime currentCount;
    register u32 saveMask;

#ifdef _DEBUG
    if (!__osViDevMgr.active) {
        __osError(ERR_OSGETTIME, 0);
        return 0;
    }
#endif

    saveMask = __osDisableInt();
    tmptime = osGetCount();
    elapseCount = tmptime - __osBaseCounter;
    currentCount = __osCurrentTime;
    __osRestoreInt(saveMask);
    return currentCount + elapseCount;
}
