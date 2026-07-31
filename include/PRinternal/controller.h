#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_

/*
 * Joybus packet layouts and the PFS-internal helpers built on them.
 *
 * The 2.0J shape of this interface is what Mickey's bytes carry, and the
 * places where the SDK changed between releases are decided here by the ROM:
 *
 *   __osPfsSelectBank (ROM 0x6DE10) takes (pfs, bank) and stores the bank into
 *   `pfs->activebank` itself; the earlier release takes the pak alone and has
 *   the caller assign the field first.
 *
 *   __OSContRamReadFormat carries `addrh`/`addrl` as separate bytes rather
 *   than one `u16 address`: __osContRamRead (ROM 0x6D9A0) writes the two with
 *   `sb` after `srl 3` and `sll 5`.
 *
 * PROVENANCE: names, layouts, constants and macros follow the N64 SDK internal
 * header as published in public decomp trees (JFG's
 * `include/PRinternal/controller.h`), a permitted source under
 * docs/CLEANROOM.md; see docs/modules.md section 1.3.
 */

#include "PR/ultratypes.h"
#include "PR/os_internal.h"
#include "PR/os_message.h"
#include "PR/os_cont.h"
#include "PR/os_pfs.h"
#include "PR/os_pi.h"
#include "PR/rcp.h"

#define CHNL_ERR(format) (((format).rxsize & CHNL_ERR_MASK) >> 4)
#define CHNL_ERR_MASK    0xC0

typedef struct {
    u32 ramarray[15];
    u32 pifstatus;
} OSPifRam;

typedef struct {
    u8 dummy;
    u8 txsize;
    u8 rxsize;
    u8 cmd;
    u8 typeh;
    u8 typel;
    u8 status;
    u8 dummy1;
} __OSContRequesFormat;

typedef struct {
    u8 txsize;
    u8 rxsize;
    u8 cmd;
    u8 typeh;
    u8 typel;
    u8 status;
} __OSContRequesFormatShort;

typedef struct {
    u8 dummy;
    u8 txsize;
    u8 rxsize;
    u8 cmd;
    u8 addrh;
    u8 addrl;
    u8 data[BLOCKSIZE];
    u8 datacrc;
} __OSContRamReadFormat;

typedef union {
    struct {
        u8 bank;
        u8 page;
    } inode_t;
    u16 ipage;
} __OSInodeUnit;

typedef struct {
    u32 game_code;
    u16 company_code;
    __OSInodeUnit start_page;
    u8 status;
    s8 reserved;
    u16 data_sum;
    u8 ext_name[PFS_FILE_EXT_LEN];
    u8 game_name[PFS_FILE_NAME_LEN];
} __OSDir;

typedef struct {
    __OSInodeUnit inode_page[128];
} __OSInode;

typedef struct {
    u32 repaired;
    u32 random;
    u64 serial_mid;
    u64 serial_low;
    u16 deviceid;
    u8 banks;
    u8 version;
    u16 checksum;
    u16 inverted_checksum;
} __OSPackId;

typedef struct {
    __OSInode inode;
    u8 bank;
    u8 map[PFS_INODE_DIST_MAP];
} __OSInodeCache;

/* Joybus commands. */
#define CONT_CMD_REQUEST_STATUS 0
#define CONT_CMD_READ_BUTTON    1
#define CONT_CMD_READ_PAK       2
#define CONT_CMD_WRITE_PAK      3
#define CONT_CMD_RESET          0xFF

#define CONT_CMD_REQUEST_STATUS_TX 1
#define CONT_CMD_READ_PAK_TX       3
#define CONT_CMD_WRITE_PAK_TX      35

#define CONT_CMD_REQUEST_STATUS_RX 3
#define CONT_CMD_READ_PAK_RX       33
#define CONT_CMD_WRITE_PAK_RX      1

#define CONT_CMD_NOP 0xFF
#define CONT_CMD_END 0xFE
#define CONT_CMD_EXE 1

#define DIR_STATUS_EMPTY    0
#define DIR_STATUS_UNKNOWN  1
#define DIR_STATUS_OCCUPIED 2

/* Accessory address space, in bytes and then in the blocks the bus takes. */
#define CONT_ADDR_DETECT 0x8000
#define CONT_ADDR_RUMBLE 0xC000

#define CONT_BLOCKS(x) ((x) / BLOCKSIZE)

#define CONT_BLOCK_DETECT CONT_BLOCKS(CONT_ADDR_DETECT)
#define CONT_BLOCK_RUMBLE CONT_BLOCKS(CONT_ADDR_RUMBLE)

extern u8 __osContLastCmd;
extern OSPifRam __osContPifRam;
extern OSPifRam __osPfsPifRam;
extern u8 __osMaxControllers;
extern s32 __osPfsLastChannel;
extern u8 __osPfsInodeCacheBank;
extern __OSInode __osPfsInodeCache;
extern s32 __osPfsInodeCacheChannel;

u16 __osSumcalc(u8 *ptr, int length);
s32 __osIdCheckSum(u16 *ptr, u16 *csum, u16 *icsum);
s32 __osRepairPackId(OSPfs *pfs, __OSPackId *badid, __OSPackId *newid);
s32 __osCheckPackId(OSPfs *pfs, __OSPackId *temp);
s32 __osGetId(OSPfs *pfs);
s32 __osCheckId(OSPfs *pfs);
s32 __osPfsRWInode(OSPfs *pfs, __OSInode *inode, u8 flag, u8 bank);
s32 __osPfsSelectBank(OSPfs *pfs, u8 bank);
s32 __osPfsDeclearPage(OSPfs *pfs, __OSInode *inode, int file_size_in_pages,
                       int *first_page, u8 bank, int *decleared, int *last_page);
s32 __osPfsReleasePages(OSPfs *pfs, __OSInode *inode, u8 start_page, u8 bank,
                        __OSInodeUnit *last_page);
s32 __osBlockSum(OSPfs *pfs, u8 page_no, u16 *sum, u8 bank);
s32 __osContRamRead(OSMesgQueue *mq, int channel, u16 address, u8 *buffer);
s32 __osContRamWrite(OSMesgQueue *mq, int channel, u16 address, u8 *buffer, int force);
void __osContGetInitData(u8 *pattern, OSContStatus *data);
void __osPackRequestData(u8 cmd);
void __osPfsRequestData(u8 cmd);
void __osPfsGetInitData(u8 *pattern, OSContStatus *data);
u8 __osContAddressCrc(u16 addr);
u8 __osContDataCrc(u8 *data);
s32 __osPfsGetStatus(OSMesgQueue *queue, int channel);

/* The SDK's own control-flow shorthands. Spelled out rather than inlined
   because IDO's output depends on the exact expression each expands to. */
#define ERRCK(fn) \
    ret = fn;     \
    if (ret != 0) \
    return ret

#define SELECT_BANK(pfs, bank) __osPfsSelectBank((pfs), (bank))

#define SET_ACTIVEBANK_TO_ZERO()          \
    if (pfs->activebank != 0) {            \
        ERRCK(__osPfsSelectBank(pfs, 0)); \
    }                                     \
    (void)0

#define PFS_CHECK_ID()                        \
    if (__osCheckId(pfs) == PFS_ERR_NEW_PACK) \
    return PFS_ERR_NEW_PACK

#define PFS_CHECK_STATUS()                    \
    if ((pfs->status & PFS_INITIALIZED) == 0) \
    return PFS_ERR_INVALID

#define PFS_GET_STATUS()                    \
    __osSiGetAccess();                      \
    ret = __osPfsGetStatus(queue, channel); \
    __osSiRelAccess();                      \
    if (ret != 0)                           \
    return ret

#endif /* _CONTROLLER_H_ */
