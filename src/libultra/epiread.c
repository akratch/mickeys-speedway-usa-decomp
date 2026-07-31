/*
 * libultra: read one word from an EPI device, taking the PI access lock.
 *
 * ROM 0x730F0-0x73140 (VRAM 0x800724F0). Byte-identical to Jet Force Gemini's
 * built libultra `io/epiread.c` object, so the file boundary is measured
 * rather than guessed -- see the provenance note in symbol_addrs.us.txt.
 *
 * WHICH OF THE PAIR THIS IS was not decided by the bytes. `epiread.c` and
 * `epiwrite.c` compile to identical instructions, and both objects match at
 * both 0x730A0 and 0x730F0 (romocc=2 in tools/find_known_objects.py). Mickey's
 * own jal target decides it: the middle call here goes to 0x80074320, which is
 * the whole-TU match on `io/epirawread.c`. See src/libultra/epiwrite.c for the
 * other half of the same argument.
 *
 * Flags: -O2 -g3 -mips2 -32. `-g3` and `-mips2` are MEASURED on this file;
 * `-O2` is NOT -- -O1 -g3 -mips2 produces byte-identical output, so these
 * bytes do not discriminate the optimisation level, and -O2 is taken from Jet
 * Force Gemini's published Makefile. The Makefile's per-file block has the
 * full twelve-combination table and keeps the two claims apart.
 *
 * What -g3 does here, structurally: without it IDO hoists the third argument's
 * spill (`sw a2,0x28(sp)`) into the first jal's delay slot -- the ROM leaves
 * that slot empty -- and emits `lw ra` before `lw v0` in the epilogue, which
 * the ROM does in the other order. The non-g3 builds are 0x44 bytes against
 * the ROM's 0x48.
 *
 * PROVENANCE: the body is N64 SDK libultra source as published in public
 * decomp trees (JFG's among them), a permitted source under docs/CLEANROOM.md;
 * see docs/modules.md section 1.3.
 */

#include "PR/os_internal.h"

s32 osEPiReadIo(OSPiHandle *handle, u32 devAddr, u32 *data) {
    s32 status;

    __osPiGetAccess();
    status = __osEPiRawReadIo(handle, devAddr, data);
    __osPiRelAccess();

    return status;
}
