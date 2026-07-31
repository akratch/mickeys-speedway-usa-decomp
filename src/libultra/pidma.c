/*
 * libultra: queue a PI DMA request with the PI manager.
 *
 * ROM 0x6FB90-0x6FC50 (VRAM 0x8006EF90). Byte-identical to Jet Force Gemini's
 * built libultra `io/pidma.c` object, so the file boundary is measured rather
 * than guessed -- see the provenance note in symbol_addrs.us.txt.
 *
 * Flags: -O2 -g3 -mips2 -32.
 *
 * PROVENANCE: the body is N64 SDK libultra source as published in public
 * decomp trees (JFG's among them), a permitted source under docs/CLEANROOM.md;
 * see docs/modules.md section 1.3.
 */

#include "PR/os_internal.h"
#include "PRinternal/piint.h"

s32 osPiStartDma(OSIoMesg *mb, s32 priority, s32 direction, u32 devAddr,
                 void *dramAddr, u32 size, OSMesgQueue *mq) {
    register s32 ret;

    if (!__osPiDevMgr.active) {
        return -1;
    }

    if (direction == OS_READ) {
        mb->hdr.type = OS_MESG_TYPE_DMAREAD;
    } else {
        mb->hdr.type = OS_MESG_TYPE_DMAWRITE;
    }

    mb->hdr.pri = priority;
    mb->hdr.retQueue = mq;
    mb->dramAddr = dramAddr;
    mb->devAddr = devAddr;
    mb->size = size;
    mb->piHandle = NULL;

    if (priority == OS_MESG_PRI_HIGH) {
        ret = osJamMesg(osPiGetCmdQueue(), (OSMesg)mb, OS_MESG_NOBLOCK);
    } else {
        ret = osSendMesg(osPiGetCmdQueue(), (OSMesg)mb, OS_MESG_NOBLOCK);
    }

    return ret;
}
