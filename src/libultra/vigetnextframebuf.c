/*
 * PROVENANCE: N64 SDK libultra source as published in the permitted Jet
 * Force Gemini decompilation. Its built whole-TU object is byte-identical
 * to Mickey; see symbol_addrs.us.txt and docs/modules.md section 1.3.
 */

#include "PR/os_internal.h"
#include "PR/ultraerror.h"
#include "PRinternal/viint.h"

void* osViGetNextFramebuffer(void) {
    register u32 saveMask;
    void* framep;

#ifdef _DEBUG
    if (!__osViDevMgr.active) {
        __osError(ERR_OSVIGETNEXTFRAMEBUFFER, 0);
        return NULL;
    }
#endif

    saveMask = __osDisableInt();
    framep = __osViNext->framep;
    __osRestoreInt(saveMask);
    return framep;
}
