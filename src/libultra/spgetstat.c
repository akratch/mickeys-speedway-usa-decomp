/*
 * libultra: read the RSP status register.
 *
 * ROM 0x74D20-0x74D30 (VRAM 0x80074120). Byte-identical to DKR's built libultra
 * `io/spgetstat.c` object, so the file boundary is measured rather than
 * guessed -- see the provenance note in symbol_addrs.us.txt.
 *
 * Flags: -O1 -mips2 -32 (see the Makefile's per-file block); libultra's io/
 * tree is built at -O1, not the project default -O2.
 */

#include "PR/rcp.h"
#include "PR/os_internal.h"

u32 __osSpGetStatus(void) {
    return IO_READ(SP_STATUS_REG);
}
