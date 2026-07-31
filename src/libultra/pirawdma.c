/*
 * libultra: start a raw PI DMA on the default domain, with no queueing.
 *
 * ROM 0x72850-0x72920 (VRAM 0x80071C50). Byte-identical to Jet Force Gemini's
 * built libultra `io/pirawdma.c` object, so the file boundary is measured
 * rather than guessed -- see the provenance note in symbol_addrs.us.txt.
 * Perfect Dark's build carries the same bytes.
 *
 * Flags: -O2 -g3 -mips2 -32.
 *
 * PROVENANCE: the body is N64 SDK libultra source as published in public
 * decomp trees (JFG's among them), a permitted source under docs/CLEANROOM.md;
 * see docs/modules.md section 1.3.
 */

#include "PRinternal/piint.h"

s32 __osPiRawStartDma(s32 direction, u32 devAddr, void *dramAddr, u32 size) {
    register u32 stat;

    WAIT_ON_IOBUSY(stat);

    IO_WRITE(PI_DRAM_ADDR_REG, osVirtualToPhysical(dramAddr));
    IO_WRITE(PI_CART_ADDR_REG, K1_TO_PHYS((u32)osRomBase | devAddr));

    switch (direction) {
        case OS_READ:
            IO_WRITE(PI_WR_LEN_REG, size - 1);
            break;
        case OS_WRITE:
            IO_WRITE(PI_RD_LEN_REG, size - 1);
            break;
        default:
            return -1;
    }
    return 0;
}
