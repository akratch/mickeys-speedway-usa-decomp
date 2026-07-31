/*
 * libultra: point the pak's bank-select block at one bank.
 *
 * ROM 0x6DE10-0x6DE90 (VRAM 0x8006D210). Byte-identical to Jet Force Gemini's
 * built libultra `io/pfsselectbank.c` object, so the file boundary is measured
 * rather than guessed -- see the provenance note in symbol_addrs.us.txt.
 *
 * Flags: -O2 -g3 -mips2 -32.
 *
 * PROVENANCE: the body is N64 SDK libultra source as published in public
 * decomp trees (JFG's among them), a permitted source under docs/CLEANROOM.md;
 * see docs/modules.md section 1.3.
 */

#include "PR/os_internal.h"
#include "PRinternal/controller.h"

s32 __osPfsSelectBank(OSPfs *pfs, u8 bank) {
    u8 temp[BLOCKSIZE];
    int i;
    s32 ret = 0;

    for (i = 0; i < BLOCKSIZE; i++) {
        temp[i] = bank;
    }

    ret = __osContRamWrite(pfs->queue, pfs->channel, CONT_BLOCK_DETECT, temp, 0);

    if (ret == 0) {
        pfs->activebank = bank;
    }

    return ret;
}
