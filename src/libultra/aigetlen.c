/*
 * libultra: how many bytes are left in the audio DMA?
 *
 * ROM 0x6FB80-0x6FB90 (VRAM 0x8006EF80). Byte-identical to DKR's built libultra
 * `io/aigetlen.c` object, so the file boundary is measured rather than
 * guessed -- see the provenance note in symbol_addrs.us.txt.
 *
 * Flags: -O1 -mips2 -32 (see the Makefile's per-file block); libultra's io/
 * tree is built at -O1, not the project default -O2.
 */

#include "PR/rcp.h"
#include "PR/os.h"

u32 osAiGetLength(void) {
    return IO_READ(AI_LEN_REG);
}
