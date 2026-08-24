/*
 * PROVENANCE: N64 SDK libultra source as published in the permitted Jet
 * Force Gemini decompilation. Its built whole-TU object is byte-identical
 * to Mickey; see symbol_addrs.us.txt and docs/modules.md section 1.3.
 */

#include "PR/os_internal.h"

extern OSIntMask __OSGlobalIntMask;	/* global interrupt mask */

void __osResetGlobalIntMask(OSHWIntr mask) {
    register u32 saveMask = __osDisableInt();

    __OSGlobalIntMask &= ~(mask & ~OS_IM_RCP);

    __osRestoreInt(saveMask);
}
