/*
 * Mickey's 0x110-byte Transfer Pak power translation unit has the official
 * SDK call surface and frame shape. The placeholder name is retained so the
 * already-integrated callers preserve their authenticated relocation target.
 *
 * PROVENANCE: adapted from the official libultra body published in the
 * Perfect Dark decomp at `src/lib/ultra/io/gbpakpower.c`, pinned in
 * `tools/reference-builds.lock`. This is a permitted published decomp source
 * under docs/CLEANROOM.md.
 */

#include "PR/os_gbpak.h"
#include "PR/os_internal.h"
#include "PRinternal/controller.h"

extern OSTimer D_800D7DE0;
extern OSMesgQueue D_800D7E00;
extern OSMesg D_800D7E18;

s32 func_8006A5A0(OSMesgQueue *mq, OSPfs *pfs, int channel);

#define MICKEY_GBPAK_POWER_DELAY ((OSTime)5625000)

s32 func_8006AC60(OSPfs *pfs, s32 flag) {
    s32 i;
    s32 ret;
    u8 buffer[BLOCKSIZE];

    for (i = 0; i < BLOCKSIZE; buffer[i++] = (u8)flag) {
        ;
    }

    ret = __osContRamWrite(pfs->queue, pfs->channel, 0x580, buffer, 0);

    if (ret == PFS_ERR_NEW_PACK) {
        ret = func_8006A5A0(pfs->queue, pfs, pfs->channel);

        if (ret == 0) {
            ret = __osContRamWrite(pfs->queue, pfs->channel, 0x580, buffer, 0);

            if (ret == PFS_ERR_NEW_PACK) {
                ret = PFS_ERR_CONTRFAIL;
            }
        }
    }

    if (flag != OS_GBPAK_POWER_OFF) {
        osSetTimer(&D_800D7DE0, MICKEY_GBPAK_POWER_DELAY, 0, &D_800D7E00,
                   &D_800D7E18);
        osRecvMesg(&D_800D7E00, NULL, OS_MESG_BLOCK);
    }

    return ret;
}
