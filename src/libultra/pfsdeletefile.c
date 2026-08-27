/*
 * libultra: Controller Pak file deletion and page accounting.
 *
 * PROVENANCE: the adapted bodies follow banjo-kazooie's
 * lib/ultralib/src/io/pfsdeletefile.c. This TU uses Mickey's measured
 * 2.0I-compatible signature set for the older page-release path.
 */

#include "PRinternal/macros.h"
#include "PR/os_internal.h"
#include "PRinternal/controller.h"

// PROVENANCE: adapted from banjo-kazooie lib/ultralib/src/io/pfsdeletefile.c:osPfsDeleteFile
s32 osPfsDeleteFile(OSPfs* pfs, u16 company_code, u32 game_code, u8* game_name, u8* ext_name) {
    s32 file_no;
    int k;
    s32 ret;
    __OSInode inode;
    __OSDir dir;
    u16 sum = 0;
    __OSInodeUnit last_page;
    u8 startpage;
    u8 bank;

    if (company_code == 0 || game_code == 0) {
        return PFS_ERR_INVALID;
    }

    PFS_CHECK_STATUS();
    PFS_CHECK_ID();
    SET_ACTIVEBANK_TO_ZERO();
    ERRCK(osPfsFindFile(pfs, company_code, game_code, game_name, ext_name, &file_no));

    if (file_no == -1) {
        return PFS_ERR_INVALID;
    }

    ERRCK(__osContRamRead(pfs->queue, pfs->channel, pfs->dir_table + file_no, (u8*)&dir));

    startpage = dir.start_page.inode_t.page;

    for (bank = dir.start_page.inode_t.bank; bank < pfs->banks;) {
        ERRCK(__osPfsRWInode(pfs, &inode, PFS_READ, bank));
        ERRCK(__osPfsReleasePages(pfs, &inode, startpage, &sum, bank, &last_page, TRUE));
        ERRCK(__osPfsRWInode(pfs, &inode, PFS_WRITE, bank));

        if (last_page.ipage == PFS_EOF) {
            break;
        }

        bank = last_page.inode_t.bank;
        startpage = last_page.inode_t.page;
    }

    if (bank >= pfs->banks) {
        return PFS_ERR_INCONSISTENT;
    }

    dir.game_code = 0;
    dir.company_code = 0;
    dir.start_page.ipage = 0;
    dir.data_sum = 0;
    for (k = 0; k < ARRLEN(dir.game_name); k++) {
        dir.game_name[k] = 0;
    }
    for (k = 0; k < ARRLEN(dir.ext_name); k++) {
        dir.ext_name[k] = 0;
    }
    dir.status = DIR_STATUS_EMPTY;

    ret = __osContRamWrite(pfs->queue, pfs->channel, pfs->dir_table + file_no, (u8*)&dir, FALSE);

    return ret;
}

#pragma GLOBAL_ASM("asm/nonmatchings/libultra/pfsdeletefile/__osPfsReleasePages.s")
#pragma GLOBAL_ASM("asm/nonmatchings/libultra/pfsdeletefile/__osBlockSum.s")
