/*
 * libultra: the PI manager's command queue, or NULL if it is not running.
 *
 * ROM 0x73AD0-0x73B00 (VRAM 0x80072ED0). Byte-identical to Jet Force Gemini's
 * built libultra `io/pigetcmdq.c` object, so the file boundary is measured
 * rather than guessed -- see the provenance note in symbol_addrs.us.txt.
 *
 * Flags: -O2 -g3 -mips2 -32, the group epiread/epiwrite established.
 *
 * PROVENANCE: the body is N64 SDK libultra source as published in public
 * decomp trees (JFG's among them), a permitted source under docs/CLEANROOM.md;
 * see docs/modules.md section 1.3.
 */

#include "PR/os_internal.h"
#include "PRinternal/piint.h"

OSMesgQueue *osPiGetCmdQueue(void) {
    if (!__osPiDevMgr.active) {
        return NULL;
    } else {
        return __osPiDevMgr.cmdQueue;
    }
}
