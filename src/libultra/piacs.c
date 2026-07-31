/*
 * libultra: the one-slot mutex serialising PI access.
 *
 * ROM 0x72730-0x72810 (VRAM 0x80071B30). Byte-identical to Jet Force Gemini's
 * built libultra `io/piacs.c` object, so the file boundary is measured rather
 * than guessed -- see the provenance note in symbol_addrs.us.txt.
 *
 * Flags: -O2 -g3 -mips2 -32.
 *
 * DEVIATION FROM THE REFERENCE: the SDK source defines the queue, its
 * one-entry buffer and the enabled flag in this translation unit. Here they
 * are `extern`: Mickey's .data and .bss are still one unsplit subsegment
 * (docs/modules.md section 6.3), so no C file can own an object yet. The
 * emitted .text is the same either way -- every reference is a %hi/%lo pair on
 * a symbol the linker resolves.
 *
 * PROVENANCE: the body is N64 SDK libultra source as published in public
 * decomp trees (JFG's among them), a permitted source under docs/CLEANROOM.md;
 * see docs/modules.md section 1.3.
 */

#include "PR/os_internal.h"
#include "PRinternal/piint.h"

#define PI_Q_BUF_LEN 1

extern OSMesg piAccessBuf[PI_Q_BUF_LEN];

void __osPiCreateAccessQueue(void) {
    __osPiAccessQueueEnabled = 1;
    osCreateMesgQueue(&__osPiAccessQueue, piAccessBuf, PI_Q_BUF_LEN);
    osSendMesg(&__osPiAccessQueue, NULL, OS_MESG_NOBLOCK);
}

void __osPiGetAccess(void) {
    OSMesg dummyMesg;

    if (!__osPiAccessQueueEnabled) {
        __osPiCreateAccessQueue();
    }
    osRecvMesg(&__osPiAccessQueue, &dummyMesg, OS_MESG_BLOCK);
}

void __osPiRelAccess(void) {
    osSendMesg(&__osPiAccessQueue, NULL, OS_MESG_NOBLOCK);
}
