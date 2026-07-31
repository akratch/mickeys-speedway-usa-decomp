#ifndef _OS_PFS_H_
#define _OS_PFS_H_

/*
 * The Controller Pak file system.
 *
 * OSPfs's offsets are read off Mickey's instructions: `osPfsFreeBlocks`
 * (ROM 0x6C750) tests `status & 1` at 0x0, loads `banks` with `lbu 0x64` and
 * `inode_start_page` with `lw 0x60`; `__osPfsSelectBank` (ROM 0x6DE10) passes
 * `queue` from `lw 0x4` and `channel` from `lw 0x8` and stores `activebank`
 * with `sb 0x65`; `osPfsNumFiles` (ROM 0x6C990) reads `dir_size` at 0x50 and
 * `dir_table` at 0x5C. The two 32-byte arrays between 0xC and 0x4C are what
 * makes those offsets come out where they do.
 *
 * PROVENANCE: names, layout and constants follow the N64 SDK header as
 * published in public decomp trees (JFG's `include/PR/os_pfs.h`), a permitted
 * source under docs/CLEANROOM.md; see docs/modules.md section 1.3.
 */

#include "PR/ultratypes.h"
#include "PR/os_message.h"

typedef struct {
    int status;
    OSMesgQueue *queue;
    int channel;
    u8 id[32];
    u8 label[32];
    int version;
    int dir_size;
    int inode_table;
    int minode_table;
    int dir_table;
    int inode_start_page;
    u8 banks;
    u8 activebank;
} OSPfs;

typedef struct {
    u32 file_size;
    u32 game_code;
    u16 company_code;
    char ext_name[4];
    char game_name[16];
} OSPfsState;

#define OS_PFS_VERSION    0x0200
#define OS_PFS_VERSION_HI (OS_PFS_VERSION >> 8)
#define OS_PFS_VERSION_LO (OS_PFS_VERSION & 255)

#define PFS_INODE_SIZE_PER_PAGE 128
#define PFS_FILE_NAME_LEN       16
#define PFS_FILE_EXT_LEN        4
#define BLOCKSIZE               32
#define PFS_ONE_PAGE            8
#define PFS_MAX_BANKS           62

/* osPfsReadWriteFile's `flag` argument. */
#define PFS_READ   0
#define PFS_WRITE  1
#define PFS_CREATE 2

/* OSPfs.status bits. */
#define PFS_INITIALIZED       0x1
#define PFS_CORRUPTED         0x2
#define PFS_ID_BROKEN         0x4
#define PFS_MOTOR_INITIALIZED 0x8
#define PFS_GBPAK_INITIALIZED 0x10

/* Page-usage sentinels stored in an inode entry. */
#define PFS_EOF            1
#define PFS_PAGE_NOT_EXIST 2
#define PFS_PAGE_NOT_USED  3

/* Error returns. */
#define PFS_ERR_NOPACK       1
#define PFS_ERR_NEW_PACK     2
#define PFS_ERR_INCONSISTENT 3
#define PFS_ERR_CONTRFAIL    CONT_OVERRUN_ERROR
#define PFS_ERR_INVALID      5
#define PFS_ERR_BAD_DATA     6
#define PFS_DATA_FULL        7
#define PFS_DIR_FULL         8
#define PFS_ERR_EXIST        9
#define PFS_ERR_ID_FATAL     10
#define PFS_ERR_DEVICE       11
#define PFS_ERR_NO_GBCART    12
#define PFS_ERR_NEW_GBCART   13

#define PFS_ID_BANK_256K 0
#define PFS_ID_BANK_1M   4
#define PFS_BANKS_256K   1

#define PFS_WRITTEN   2
#define DEF_DIR_PAGES 2

#define PFS_ID_0AREA   1
#define PFS_ID_1AREA   3
#define PFS_ID_2AREA   4
#define PFS_ID_3AREA   6
#define PFS_LABEL_AREA 7
#define PFS_ID_PAGE    (PFS_ONE_PAGE * 0)

#define PFS_BANK_LAPPED_BY  8
#define PFS_SECTOR_PER_BANK 32
#define PFS_INODE_DIST_MAP  (PFS_BANK_LAPPED_BY * PFS_SECTOR_PER_BANK)
#define PFS_SECTOR_SIZE     (PFS_INODE_SIZE_PER_PAGE / PFS_SECTOR_PER_BANK)

s32 osPfsInitPak(OSMesgQueue *queue, OSPfs *pfs, int channel);
s32 osPfsRepairId(OSPfs *pfs);
s32 osPfsInit(OSMesgQueue *queue, OSPfs *pfs, int channel);
s32 osPfsChecker(OSPfs *pfs);
s32 osPfsAllocateFile(OSPfs *pfs, u16 company_code, u32 game_code, u8 *game_name,
                      u8 *ext_name, int file_size_in_bytes, s32 *file_no);
s32 osPfsFindFile(OSPfs *pfs, u16 company_code, u32 game_code, u8 *game_name,
                  u8 *ext_name, s32 *file_no);
s32 osPfsDeleteFile(OSPfs *pfs, u16 company_code, u32 game_code, u8 *game_name,
                    u8 *ext_name);
s32 osPfsReadWriteFile(OSPfs *pfs, s32 file_no, u8 flag, int offset, int size_in_bytes,
                       u8 *data_buffer);
s32 osPfsFileState(OSPfs *pfs, s32 file_no, OSPfsState *state);
s32 osPfsIsPlug(OSMesgQueue *queue, u8 *pattern);
s32 osPfsFreeBlocks(OSPfs *pfs, s32 *bytes_not_used);
s32 osPfsNumFiles(OSPfs *pfs, s32 *max_files, s32 *files_used);

#endif /* _OS_PFS_H_ */
