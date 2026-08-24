/*
 * PROVENANCE: N64 SDK libultra source as published in the permitted Jet
 * Force Gemini decompilation. Its built whole-TU object is byte-identical
 * to Mickey; see symbol_addrs.us.txt and docs/modules.md section 1.3.
 */

#include "PR/os_internal.h"
#include "PR/ultraerror.h"
#include "PRinternal/osint.h"

void osSetTime(OSTime time) {

#ifdef _DEBUG
    if (!__osViDevMgr.active) {
        __osError(ERR_OSSETTIME, 0);
        return;
    }
#endif

    __osCurrentTime = time;
}
