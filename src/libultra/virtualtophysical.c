/*
 * PROVENANCE: N64 SDK libultra source as published in the permitted Jet
 * Force Gemini decompilation. Its built whole-TU object is byte-identical
 * to Mickey; see symbol_addrs.us.txt and docs/modules.md section 1.3.
 */

#include "PR/os_internal.h"
#include "PR/R4300.h"
#include "PRinternal/osint.h"

u32 osVirtualToPhysical(void* addr) {
    if (IS_KSEG0(addr)) {
        return K0_TO_PHYS(addr);
    } else if (IS_KSEG1(addr)) {
        return K1_TO_PHYS(addr);
    } else {
        return __osProbeTLB(addr);
    }
}
