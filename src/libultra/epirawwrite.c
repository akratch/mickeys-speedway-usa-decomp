/*
 * libultra: write one word to an EPI device, syncing its bus timings first.
 *
 * ROM 0x74DC0-0x74F20 (VRAM 0x800741C0). Byte-identical to Jet Force Gemini's
 * built libultra `io/epirawwrite.c` object, so the file boundary is measured
 * rather than guessed -- see the provenance note in symbol_addrs.us.txt.
 *
 * Flags: -O2 -g3 -mips2 -32.
 *
 * PROVENANCE: the body is N64 SDK libultra source as published in public
 * decomp trees (JFG's among them), a permitted source under docs/CLEANROOM.md;
 * see docs/modules.md section 1.3.
 */

#include "PRinternal/piint.h"

s32 __osEPiRawWriteIo(OSPiHandle *pihandle, u32 devAddr, u32 data) {
    register u32 stat;
    register u32 domain;

    EPI_SYNC(pihandle, stat, domain);
    IO_WRITE(pihandle->baseAddress | devAddr, data);

    return 0;
}
