/*
 * libultra: the pak's directory capacity, and how much of it is in use.
 *
 * ROM 0x6C990-0x6CAC0 (VRAM 0x8006BD90). Byte-identical to Jet Force Gemini's
 * built libultra `io/pfsnumfiles.c` object, so the file boundary is measured
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

s32 osPfsNumFiles(OSPfs *pfs, s32 *max_files, s32 *files_used) {
    int j;
    s32 ret;
    __OSDir dir;
    int files = 0;

    PFS_CHECK_STATUS();
    ERRCK(__osCheckId(pfs));
    SET_ACTIVEBANK_TO_ZERO();

    for (j = 0; j < pfs->dir_size; j++) {
        ERRCK(__osContRamRead(pfs->queue, pfs->channel, pfs->dir_table + j, (u8 *)&dir));

        if (dir.company_code != 0 && dir.game_code != 0) {
            files++;
        }
    }
    *files_used = files;
    *max_files = pfs->dir_size;

    ret = __osPfsGetStatus(pfs->queue, pfs->channel);
    return ret;
}
