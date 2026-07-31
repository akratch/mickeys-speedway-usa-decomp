/*
 * libultra: write one word to an EPI device, taking the PI access lock.
 *
 * ROM 0x730A0-0x730F0 (VRAM 0x800724A0). Byte-identical to Jet Force Gemini's
 * built libultra `io/epiwrite.c` object, so the file boundary is measured
 * rather than guessed -- see the provenance note in symbol_addrs.us.txt.
 *
 * WHICH OF THE PAIR THIS IS: see src/libultra/epiread.c. The bytes cannot tell
 * these two apart; the middle call here goes to 0x800741C0, the whole-TU match
 * on `io/epirawwrite.c`, and that is what settles it.
 *
 * Flags: -O2 -g3 -mips2 -32, measured separately from epiread.c rather than
 * inherited from it, and with the same split: `-g3 -mips2` measured, `-O2`
 * borrowed from JFG's Makefile and not discriminated by these bytes. See
 * src/libultra/epiread.c and the Makefile's per-file block.
 *
 * PROVENANCE: the body is N64 SDK libultra source as published in public
 * decomp trees (JFG's among them), a permitted source under docs/CLEANROOM.md;
 * see docs/modules.md section 1.3.
 */

#include "PR/os_internal.h"

s32 osEPiWriteIo(OSPiHandle *handle, u32 devAddr, u32 data) {
    s32 status;

    __osPiGetAccess();
    status = __osEPiRawWriteIo(handle, devAddr, data);
    __osPiRelAccess();

    return status;
}
