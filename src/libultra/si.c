/*
 * libultra: is the serial interface busy?
 *
 * ROM 0x75DE0-0x75E10 (VRAM 0x800751E0). Byte-identical to DKR's built libultra
 * `io/si.c` object, so the file boundary is measured rather than
 * guessed -- see the provenance note in symbol_addrs.us.txt.
 *
 * Flags: -O1 -mips2 -32 (see the Makefile's per-file block). libultra's io/
 * tree is built at -O1, not the project default -O2; at -O2 IDO folds the
 * frame away and the byte count no longer matches.
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
