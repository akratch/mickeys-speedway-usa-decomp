/*
 * Mickey's 0x230-byte Transfer Pak initialization translation unit has the
 * official SDK call surface and state layout. The placeholder name is retained
 * so the already-integrated callers preserve their authenticated relocation
 * target. Its timer objects stay in the existing authenticated BSS range.
 *
 * PROVENANCE: adapted from the official libultra body published in the
 * Perfect Dark decomp at `src/lib/ultra/io/gbpakinit.c`, pinned in
 * `tools/reference-builds.lock`. This is a permitted published decomp source
 * under docs/CLEANROOM.md.
 */

#include "PR/os_gbpak.h"
#include "PR/os_internal.h"
#include "PRinternal/controller.h"

extern OSTimer D_800D7DE0;
extern OSMesgQueue D_800D7E00;
extern OSMesg D_800D7E18;

#define MICKEY_GBPAK_INIT_DELAY ((OSTime)9000000)

s32 func_8006A5A0(OSMesgQueue *queue, OSPfs *pfs, int channel) {
    s32 ret;
    s32 i;
    u8 buffer[BLOCKSIZE];

    pfs->status = 0;

    for (i = 0; i < BLOCKSIZE; i++) {
        buffer[i] = 0xFE;
    }

    ret = __osContRamWrite(queue, channel, 0x400, buffer, 0);

    if (ret == PFS_ERR_NEW_PACK) {
        ret = __osContRamWrite(queue, channel, 0x400, buffer, 0);
    }

    if (ret != 0) {
        return ret;
    }

    ret = __osContRamRead(queue, channel, 0x400, buffer);

    if (ret == PFS_ERR_NEW_PACK) {
        ret = PFS_ERR_CONTRFAIL;
    }

    if (ret != 0) {
        return ret;
    }

    if (buffer[BLOCKSIZE - 1] == 0xFE) {
        return PFS_ERR_DEVICE;
    }

    for (i = 0; i < BLOCKSIZE; i++) { buffer[i] = 0x84; }

    ret = __osContRamWrite(queue, channel, 0x400, buffer, 0);

    if (ret == PFS_ERR_NEW_PACK) {
        ret = PFS_ERR_CONTRFAIL;
    }

    if (ret != 0) {
        return ret;
    }

    ret = __osContRamRead(queue, channel, 0x400, buffer);

    if (ret == PFS_ERR_NEW_PACK) {
        ret = PFS_ERR_CONTRFAIL;
    }

    if (ret != 0) {
        return ret;
    }

    if (buffer[BLOCKSIZE - 1] != 0x84) {
        return PFS_ERR_DEVICE;
    }

    ret = __osPfsGetStatus(queue, channel);

    if (ret != 0) {
        return ret;
    }

    osCreateMesgQueue(&D_800D7E00, &D_800D7E18, 1);
    osSetTimer(&D_800D7DE0, MICKEY_GBPAK_INIT_DELAY, 0, &D_800D7E00,
               &D_800D7E18);
    osRecvMesg(&D_800D7E00, NULL, OS_MESG_BLOCK);

    pfs->queue = queue;
    pfs->status = PFS_GBPAK_INITIALIZED;
    pfs->channel = channel;
    pfs->activebank = 0x84;
    pfs->banks = 0xFF;
    pfs->version = 0xFF;
    pfs->dir_size = 0xFF;

    return 0;
}
