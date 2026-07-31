/*
 * libultra: write the RDP command status register.
 *
 * ROM 0x70AE0-0x70AF0 (VRAM 0x8006FEE0). Byte-identical to DKR's built libultra
 * `io/dpsetstat.c` object, so the file boundary is measured rather than
 * guessed -- see the provenance note in symbol_addrs.us.txt.
 *
 * Flags: -O1 -mips2 -32 (see the Makefile's per-file block); libultra's io/
 * tree is built at -O1, not the project default -O2.
 */

#include "PR/rcp.h"
#include "PR/os.h"

void osDpSetStatus(u32 status) {
    IO_WRITE(DPC_STATUS_REG, status);
}
