/*
 * libultra: is the RSP's DMA engine busy?
 *
 * ROM 0x74CF0-0x74D20 (VRAM 0x800740F0). Byte-identical to DKR's built
 * libultra `io/sp.c` object, so the file boundary is measured, not guessed --
 * see the provenance note in symbol_addrs.us.txt.
 *
 * Builds with the project default flags (-O2 -mips1 -32): no branch-likely
 * instructions here, so unlike src/libultra/string.c this needs no override.
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
