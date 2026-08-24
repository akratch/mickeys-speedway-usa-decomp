/*
 * PROVENANCE: N64 SDK libultra source as published in the permitted Jet
 * Force Gemini decompilation. Its built whole-TU object is byte-identical
 * to Mickey; see symbol_addrs.us.txt and docs/modules.md section 1.3.
 */

#include "PR/os_internal.h"
#include "PRinternal/viint.h"

// TODO: this comes from a header
#ident "$Revision: 1.17 $"

void osViBlack(u8 active) {
    register u32 saveMask = __osDisableInt();

    if (active) {
        __osViNext->state |= VI_STATE_BLACK;
    } else {
        __osViNext->state &= ~VI_STATE_BLACK;
    }

    __osRestoreInt(saveMask);
}
