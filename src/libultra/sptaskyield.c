/*
 * libultra: ask the running RSP task to yield.
 *
 * ROM 0x71E80-0x71EA0 (VRAM 0x80071280). Byte-identical to DKR's built
 * libultra `io/sptaskyield.c` object, so the file boundary is measured rather
 * than guessed -- see the provenance note in symbol_addrs.us.txt.
 *
 * First compiled C in this project to carry a relocation of its own: the call
 * below is an R_MIPS_26 that the linker resolves against __osSpSetStatus,
 * which src/libultra/spsetstat.c defines.
 *
 * Flags: -O1 -mips2 -32 (libultra's io/ tree; see the Makefile).
 */

#include "PR/rcp.h"
#include "PR/os_internal.h"

void osSpTaskYield(void) {
    __osSpSetStatus(SP_SET_SIG0);
}
