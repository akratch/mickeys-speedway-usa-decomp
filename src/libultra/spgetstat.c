/*
 * libultra: read the RSP status register.
 *
 * ROM 0x74D20-0x74D30 (VRAM 0x80074120). Byte-identical to DKR's built libultra
 * `io/spgetstat.c` object, so the file boundary is measured rather than
 * guessed -- see the provenance note in symbol_addrs.us.txt.
 *
 * Flags: -O1 -mips2 -32, measured on this file, not inherited from any rule
 * about libultra's io/ tree -- see the Makefile's per-file block.
 *
 * PROVENANCE: the body is N64 SDK libultra source as published in public
 * decomp trees (DKR's among them), a permitted source under docs/CLEANROOM.md;
 * see docs/modules.md section 1.3.
 */

#include "PR/rcp.h"
#include "PR/os_internal.h"

u32 __osSpGetStatus(void) {
    return IO_READ(SP_STATUS_REG);
}
