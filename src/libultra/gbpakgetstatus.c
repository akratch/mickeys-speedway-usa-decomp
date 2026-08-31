/*
 * libultra Transfer Pak status reader.
 *
 * Perfect Dark's built `lib/ultra/io/gbpakgetstatus.o` is the unique
 * whole-object correspondence for Mickey's 0x180-byte translation unit.
 * This source uses that object's measured default loop-unroll mode with
 * -O2 -mips2 -32.
 *
 * DEVIATION FROM THE REFERENCE: the initialization call retains Mickey's
 * current tier-D placeholder `func_8006A5A0`. Its target corresponds to
 * Perfect Dark's `osGbpakInit`, but that target is not itself a known-object
 * match and therefore does not yet justify adopting the SDK name.
 *
 * PROVENANCE: adapted from the official libultra body published in the
 * Banjo-Kazooie decomp at `lib/ultralib/src/io/gbpakgetstatus.c`; Perfect
 * Dark's `src/lib/ultra/io/gbpakgetstatus.c` and built object authenticate
 * the TU identity and flag group. Both are permitted published decomp sources
 * under docs/CLEANROOM.md. Mickey's bytes decide the local buffer spelling,
 * flags, and call target.
 */

#include "PR/os_gbpak.h"
#include "PRinternal/controller.h"

s32 func_8006A5A0(OSMesgQueue *mq, OSPfs *pfs, int channel);

s32 osGbpakGetStatus(OSPfs *pfs, u8 *status) {
    s32 ret;
    s32 i;
    u32 buffer[BLOCKSIZE / sizeof(u32)];

    ret = __osContRamRead(pfs->queue, pfs->channel, 0x400, (u8 *)buffer);

    if ((ret == PFS_ERR_NEW_PACK) ||
        (((u8 *)buffer)[BLOCKSIZE - 1] !=
         (OS_GBPAK_RSTB_DETECTION | OS_GBPAK_GBCART_ON))) {
        ERRCK(func_8006A5A0(pfs->queue, pfs, pfs->channel));
    }

    ret = __osContRamRead(pfs->queue, pfs->channel, 0x580, (u8 *)buffer);
    if (ret == 0) {
        ERRCK(__osPfsGetStatus(pfs->queue, pfs->channel));

        *status = ((u8 *)buffer)[0];

        for (i = 1; i < BLOCKSIZE; i++) {
            *status |= ((u8 *)buffer)[i];
        }

        *status &= (OS_GBPAK_RSTB_DETECTION | OS_GBPAK_GBCART_PULL);
        *status |= ((u8 *)buffer)[BLOCKSIZE - 1];

        if (!(*status & OS_GBPAK_GBCART_ON)) {
            ret = PFS_ERR_NO_GBCART;
        } else if (*status & OS_GBPAK_GBCART_PULL) {
            ret = PFS_ERR_NEW_GBCART;
        }
    } else if (ret == 2) {
        ret = PFS_ERR_CONTRFAIL;
    }
    return ret;
}
