/*
 * libultra Transfer Pak bank selector.
 *
 * Perfect Dark's built `lib/ultra/io/gbpakselectbank.o` is the unique
 * whole-object match for Mickey's 0xD0-byte text. The loop-shaped SDK source
 * reproduces all 52 words with -O2 -mips2 -32 -Wo,-loopunroll,0.
 *
 * DEVIATION FROM THE REFERENCE: the init call at text offset 0x78 retains
 * Mickey's current tier-D placeholder `func_8006A5A0`. Its target corresponds
 * to Perfect Dark's `osGbpakInit`, but the target TU is not itself a
 * known-object match and therefore does not yet justify adopting that name.
 *
 * PROVENANCE: adapted from Perfect Dark's
 * `src/lib/ultra/io/gbpakselectbank.c`, a permitted published decomp source
 * under docs/CLEANROOM.md. Mickey's bytes decide the local loop spelling,
 * flags, and call target.
 */

#include "PR/os_gbpak.h"
#include "PRinternal/controller.h"

s32 func_8006A5A0(OSMesgQueue *mq, OSPfs *pfs, int channel);

s32 __osGbpakSelectBank(OSPfs *pfs, u8 bank) {
    s32 ret;
    s32 i;
    u8 buffer[8][4];

    if (bank >= 3) {
        return PFS_ERR_INVALID;
    }

    for (i = 0; i < 8; i++) {
        buffer[i][0] = bank;
        buffer[i][1] = bank;
        buffer[i][2] = bank;
        buffer[i][3] = bank;
    }

    ret = __osContRamWrite(pfs->queue, pfs->channel, 0x500, (u8 *)buffer, 0);

    if (ret == PFS_ERR_NEW_PACK) {
        ret = func_8006A5A0(pfs->queue, pfs, pfs->channel);

        if (ret == 0) {
            ret = __osContRamWrite(pfs->queue, pfs->channel, 0x500,
                                   (u8 *)buffer, 0);

            if (ret == PFS_ERR_NEW_PACK) {
                ret = PFS_ERR_CONTRFAIL;
            }
        }
    }

    if (ret == 0) {
        pfs->banks = bank;
    }

    return ret;
}
