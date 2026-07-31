/*
 * libultra: is the RDP's DMA engine busy?
 *
 * ROM 0x74D30-0x74D60 (VRAM 0x80074130). Byte-identical to DKR's built libultra
 * `io/dp.c` object, so the file boundary is measured rather than
 * guessed -- see the provenance note in symbol_addrs.us.txt.
 *
 * Flags: -O1 -mips2 -32 (see the Makefile's per-file block). libultra's io/
 * tree is built at -O1, not the project default -O2; at -O2 IDO folds the
 * frame away and the byte count no longer matches.
 */

#include "PR/rcp.h"
#include "PR/os_internal.h"

s32 __osDpDeviceBusy(void) {
    register u32 status;

    status = IO_READ(DPC_STATUS_REG);
    if (status & (DPC_STATUS_DMA_BUSY)) {
        return 1;
    }
    return 0;
}
