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

void osViSwapBuffer(void* frameBufPtr) {
    u32 saveMask;

#ifdef _DEBUG
    if (!__osViDevMgr.active) {
        __osError(ERR_OSVISWAPBUFFER_VIMGR, 0);
        return;
    }

    assert(frameBufPtr != NULL);

    if ((u32)frameBufPtr & 0x3f) {
        __osError(ERR_OSVISWAPBUFFER_ADDR, 1, frameBufPtr);
        return;
    }
#endif

    saveMask = __osDisableInt();

    __osViNext->framep = frameBufPtr;
    __osViNext->state |= VI_STATE_BUFFER_UPDATED;
    __osRestoreInt(saveMask);
}
