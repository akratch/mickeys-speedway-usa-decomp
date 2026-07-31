/*
 * libultra: bring a Controller Pak into a usable state on one channel.
 *
 * ROM 0x6C8F0-0x6C990 (VRAM 0x8006BCF0). Byte-identical to Jet Force Gemini's
 * built libultra `io/pfsinit.c` object, so the file boundary is measured
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
#include "PRinternal/siint.h"

s32 osPfsInit(OSMesgQueue *queue, OSPfs *pfs, int channel) {
    s32 ret = 0;

    __osSiGetAccess();
    ret = __osPfsGetStatus(queue, channel);
    __osSiRelAccess();

    if (ret != 0) {
        return ret;
    }

    pfs->queue = queue;
    pfs->channel = channel;
    pfs->status = 0;
    pfs->activebank = -1;
    ERRCK(__osGetId(pfs));

    ret = osPfsChecker(pfs);
    pfs->status |= PFS_INITIALIZED;
    return ret;
}
