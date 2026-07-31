/*
 * libultra: how many bytes of the pak are unallocated.
 *
 * ROM 0x6C750-0x6C8F0 (VRAM 0x8006BB50). Byte-identical to Jet Force Gemini's
 * built libultra `io/pfsfreeblocks.c` object, so the file boundary is measured
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

s32 osPfsFreeBlocks(OSPfs *pfs, s32 *bytes_not_used) {
    int j;
    int pages = 0;
    __OSInode inode;
    s32 ret = 0;
    u8 bank;
    int offset;

    PFS_CHECK_STATUS();
    ERRCK(__osCheckId(pfs));

    for (bank = 0; bank < pfs->banks; bank++) {
        ERRCK(__osPfsRWInode(pfs, &inode, PFS_READ, bank));
        offset = ((bank > 0) ? 1 : pfs->inode_start_page);

        for (j = offset; j < 128; j++) {
            if (inode.inode_page[j].ipage == PFS_PAGE_NOT_USED) {
                pages++;
            }
        }
    }

    *bytes_not_used = pages * PFS_ONE_PAGE * BLOCKSIZE;
    return 0;
}
