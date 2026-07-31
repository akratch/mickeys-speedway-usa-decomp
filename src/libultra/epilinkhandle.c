/*
 * libultra: push an EPI handle onto the front of the PI handle table.
 *
 * ROM 0x73050-0x730A0 (VRAM 0x80072450). Byte-identical to Jet Force Gemini's
 * built libultra `io/epilinkhandle.c` object, so the file boundary is measured
 * rather than guessed -- see the provenance note in symbol_addrs.us.txt.
 *
 * Flags: -O2 -g3 -mips2 -32.
 *
 * PROVENANCE: the body is N64 SDK libultra source as published in public
 * decomp trees (JFG's among them), a permitted source under docs/CLEANROOM.md;
 * see docs/modules.md section 1.3.
 */

#include "PRinternal/piint.h"

s32 osEPiLinkHandle(OSPiHandle *EPiHandle) {
    u32 saveMask = __osDisableInt();

    EPiHandle->next = __osPiTable;
    __osPiTable = EPiHandle;

    __osRestoreInt(saveMask);
    return 0;
}
