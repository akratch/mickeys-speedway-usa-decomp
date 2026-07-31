/*
 * libultra: is the RSP's DMA engine busy?
 *
 * ROM 0x74CF0-0x74D20 (VRAM 0x800740F0). Byte-identical to DKR's built
 * libultra `io/sp.c` object, so the file boundary is measured, not guessed --
 * see the provenance note in symbol_addrs.us.txt.
 *
 * Flags: -O1 -mips2 -32, overriding the project default -O2 -mips1 -32 (see the
 * Makefile's per-file block). Both halves are forced, and this is the file they
 * were measured on: at -O2 IDO folds the `addiu sp,sp,-8 / addiu sp,sp,8` frame
 * away entirely and the .text comes out the wrong size, and -mips2 is needed for
 * the same branch-likely reason as src/libultra/string.c. The `register` on the
 * local is load-bearing too -- without it -O1 spills the status word to the
 * stack, which the ROM does not do (4 differing words, zero with it).
 */

#include "PR/rcp.h"
#include "PR/os_internal.h"

s32 __osSpDeviceBusy(void) {
    register u32 status;

    status = IO_READ(SP_STATUS_REG);
    if (status & (SP_STATUS_IO_FULL | SP_STATUS_DMA_FULL | SP_STATUS_DMA_BUSY)) {
        return 1;
    }
    return 0;
}
