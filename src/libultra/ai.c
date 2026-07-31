/*
 * libultra: is the audio interface's DMA FIFO full?
 *
 * ROM 0x73890-0x738C0 (VRAM 0x80072C90). Byte-identical to DKR's built libultra
 * `io/ai.c` object, so the file boundary is measured rather than
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

s32 __osAiDeviceBusy(void) {
    register u32 status;

    status = IO_READ(AI_STATUS_REG);
    if (status & (AI_STATUS_FIFO_FULL)) {
        return 1;
    }
    return 0;
}
