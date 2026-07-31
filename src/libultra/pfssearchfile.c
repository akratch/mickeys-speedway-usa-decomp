/*
 * libultra: find a file in the pak's directory by company/game and name.
 *
 * ROM 0x6CCA0-0x6CE70 (VRAM 0x8006C0A0). Byte-identical to Jet Force Gemini's
 * built libultra `io/pfssearchfile.c` object, so the file boundary is measured
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

s32 osPfsFindFile(OSPfs *pfs, u16 company_code, u32 game_code, u8 *game_name,
                  u8 *ext_name, s32 *file_no) {
    s32 j;
    int i;
    __OSDir dir;
    s32 ret = 0;
    int fail;

    if (!(pfs->status & PFS_INITIALIZED)) {
        return PFS_ERR_INVALID;
    }
    ERRCK(__osCheckId(pfs));

    for (j = 0; j < pfs->dir_size; j++) {
        ERRCK(__osContRamRead(pfs->queue, pfs->channel, pfs->dir_table + j, (u8 *)&dir));
        ERRCK(__osPfsGetStatus(pfs->queue, pfs->channel));

        if ((dir.company_code == company_code) && dir.game_code == game_code) {
            fail = FALSE;

            if (game_name != NULL) {
                for (i = 0; i < PFS_FILE_NAME_LEN; i++) {
                    if (dir.game_name[i] != game_name[i]) {
                        fail = TRUE;
                        break;
                    }
                }
            }

            if (ext_name != NULL && !fail) {
                for (i = 0; i < PFS_FILE_EXT_LEN; i++) {
                    if (dir.ext_name[i] != ext_name[i]) {
                        fail = TRUE;
                        break;
                    }
                }
            }

            if (!fail) {
                *file_no = j;
                return ret;
            }
        }
    }

    *file_no = -1;
    return PFS_ERR_INVALID;
}
