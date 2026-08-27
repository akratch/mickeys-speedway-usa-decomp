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

// PROVENANCE: adapted from banjo-kazooie lib/ultralib/src/io/pfsdeletefile.c:__osPfsReleasePages
s32 __osPfsReleasePages(OSPfs *pfs, __OSInode *inode, u8 start_page, u16 *sum, u8 bank, __OSInodeUnit *last_page, int flag)
{
    __OSInodeUnit next_page;
    __OSInodeUnit old_page;
    s32 ret;
    int offset;
    ret = 0;
    next_page = inode->inode_page[start_page];

    if (next_page.ipage != 1) {
        offset = (next_page.inode_t.bank > 0) ? 1 : pfs->inode_start_page;
    } else {
        offset = (bank > 0) ? 1 : pfs->inode_start_page;
    }

    if (next_page.inode_t.page < offset && next_page.ipage != 1) {
        return PFS_ERR_INCONSISTENT;
    }

    *last_page = next_page;

    if (flag == TRUE) {
        inode->inode_page[start_page].ipage = 3;
    }

    ERRCK(__osBlockSum(pfs, start_page, sum, bank));

    if (next_page.ipage == 1) {
        return 0;
    }

    while (next_page.ipage >= pfs->inode_start_page) {
        old_page = next_page;
        next_page = inode->inode_page[next_page.inode_t.page];
        inode->inode_page[old_page.inode_t.page].ipage = 3;

        ERRCK(__osBlockSum(pfs, old_page.inode_t.page, sum, bank));

        if (next_page.inode_t.bank != bank) {
            break;
        }
    }

#ifdef RAREDIFFS
    if (next_page.ipage >= pfs->inode_start_page) {
#else
    if (next_page.ipage >= pfs->inode_start_page && next_page.inode_t.bank == bank) {
#endif
        inode->inode_page[next_page.inode_t.page].ipage = 3;
    }

    *last_page = next_page;
    return 0;
}
#pragma GLOBAL_ASM("asm/nonmatchings/libultra/pfsdeletefile/__osBlockSum.s")
