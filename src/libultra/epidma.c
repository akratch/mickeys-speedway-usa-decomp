/*
 * libultra: queue an EPI DMA request with the PI manager.
 *
 * ROM 0x73140-0x731E0 (VRAM 0x80072540). Byte-identical to Jet Force Gemini's
 * built libultra `io/epidma.c` object, so the file boundary is measured rather
 * than guessed -- see the provenance note in symbol_addrs.us.txt.
 *
 * Flags: -O2 -g3 -mips2 -32.
 *
 * PROVENANCE: the body is N64 SDK libultra source as published in public
 * decomp trees (JFG's among them), a permitted source under docs/CLEANROOM.md;
 * see docs/modules.md section 1.3.
 */

#include "PRinternal/piint.h"

s32 osEPiStartDma(OSPiHandle *pihandle, OSIoMesg *mb, s32 direction) {
    register s32 ret;

    if (!__osPiDevMgr.active) {
        return -1;
    }

    mb->piHandle = pihandle;

    if (direction == OS_READ) {
        mb->hdr.type = OS_MESG_TYPE_EDMAREAD;
    } else {
        mb->hdr.type = OS_MESG_TYPE_EDMAWRITE;
    }

    if (mb->hdr.pri == OS_MESG_PRI_HIGH) {
        ret = osJamMesg(osPiGetCmdQueue(), (OSMesg)mb, OS_MESG_NOBLOCK);
    } else {
        ret = osSendMesg(osPiGetCmdQueue(), (OSMesg)mb, OS_MESG_NOBLOCK);
    }

    return ret;
}
