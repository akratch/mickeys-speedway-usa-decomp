/*
 * libultra: is the serial interface busy?
 *
 * ROM 0x75DE0-0x75E10 (VRAM 0x800751E0). Byte-identical to DKR's built libultra
 * `io/si.c` object, so the file boundary is measured rather than
 * guessed -- see the provenance note in symbol_addrs.us.txt.
 *
 * Flags: -O1 -mips2 -32, measured on this file (see the Makefile's per-file
 * block). The grouping there is "TUs measured to need these flags", not
 * "libultra's io/ tree": at -O2 IDO folds the frame away entirely and the byte
 * count no longer matches.
 *
 * PROVENANCE: the body is N64 SDK libultra source as published in public
 * decomp trees (DKR's among them), a permitted source under docs/CLEANROOM.md;
 * see docs/modules.md section 1.3.
 */

#include "PR/rcp.h"
#include "PR/os_internal.h"

s32 __osSiDeviceBusy(void) {
    register u32 status;

    status = IO_READ(SI_STATUS_REG);
    if (status & (SI_STATUS_DMA_BUSY | SI_STATUS_RD_BUSY)) {
        return 1;
    }
    return 0;
}
