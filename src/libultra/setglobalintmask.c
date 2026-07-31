/*
 * libultra: OR bits into the global hardware interrupt mask.
 *
 * ROM 0x75080-0x750D0 (VRAM 0x80074480). Byte-identical to Jet Force Gemini's
 * built libultra `os/setglobalintmask.c` object, so the file boundary is
 * measured rather than guessed -- see the provenance note in
 * symbol_addrs.us.txt. Perfect Dark's and Banjo-Kazooie's builds carry the
 * same bytes.
 *
 * Flags: -O1 -mips2 -32, with `uopt` forced to run at -O1
 * (-Xphase,uopt,+ -Xphase,uopt,-O1 through tools/ido/ido-phases.py).
 *
 * MEASURED, and the reason this file is the tree's only wrapper user: the ROM
 * fills the first jal's delay slot with the callee-save `sw s0, 0x18($sp)`.
 * `tools/ido/cc` runs `uopt` only at -O2 and above, so at -O1 the optimiser
 * never ran, the slot stayed empty and the function came out 20 instructions
 * instead of 19 -- none of the twenty-two {-O0,-O1,-O2} x {-g,-g1,-g2,-g3,
 * none} x {-mips1,-mips2} driver combinations could close it, because none of
 * them could put uopt and -O1 together. Driving the phases directly does: 19
 * of 19 words agree and the only differences are the six the linker fills in
 * (R_MIPS_26 on __osDisableInt/__osRestoreInt, two %hi/%lo pairs on
 * __OSGlobalIntMask). -g3 is irrelevant here; -mips2 is required.
 *
 * PROVENANCE: the body is N64 SDK libultra source as published in public
 * decomp trees (JFG's among them), a permitted source under docs/CLEANROOM.md;
 * see docs/modules.md section 1.3.
 */

#include "PR/ultratypes.h"
#include "PR/os_internal.h"

void __osSetGlobalIntMask(u32 mask) {
    register u32 saveMask = __osDisableInt();

    __OSGlobalIntMask |= mask;
    __osRestoreInt(saveMask);
}
