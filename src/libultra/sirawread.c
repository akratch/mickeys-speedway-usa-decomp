/*
 * PROVENANCE: N64 SDK libultra source as published in the permitted Jet
 * Force Gemini decompilation. Its built whole-TU object is byte-identical
 * to Mickey; see symbol_addrs.us.txt and docs/modules.md section 1.3.
 */

#include "PR/os_internal.h"
#include "PR/rcp.h"
#include "assert.h"
#include "PRinternal/siint.h"








































// Adjust line numbers to match assert
#if BUILD_VERSION < VERSION_J
#line 45
#endif

// TODO: this comes from a header
#ident "$Revision: 1.17 $"

s32 __osSiRawReadIo(u32 devAddr, u32* data) {
    assert((devAddr & 0x3) == 0);
    assert(data != NULL);

    if (__osSiDeviceBusy()) {
        return -1;
    }

    *data = IO_READ(devAddr);
    return 0;
}
