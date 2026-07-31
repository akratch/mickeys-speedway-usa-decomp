/*
 * libultra: is the audio interface's DMA FIFO full?
 *
 * ROM 0x73890-0x738C0 (VRAM 0x80072C90). Byte-identical to DKR's built libultra
 * `io/ai.c` object, so the file boundary is measured rather than
 * guessed -- see the provenance note in symbol_addrs.us.txt.
 *
 * Flags: -O1 -mips2 -32 (see the Makefile's per-file block). libultra's io/
 * tree is built at -O1, not the project default -O2; at -O2 IDO folds the
 * frame away and the byte count no longer matches.
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
