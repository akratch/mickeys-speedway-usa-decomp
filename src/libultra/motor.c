/*
 * libultra Rumble Pak driver.
 *
 * Jet Force Gemini's built `libultra/src/io/motor.c.o` is the unique
 * whole-object match for Mickey's 0x410-byte text. Flags: -O2 -g3 -mips2 -32,
 * BUILD_VERSION=VERSION_J, with JFGDIFFS.
 *
 * DEVIATION FROM THE REFERENCE: JFG defines the static 0x100-byte
 * __MotorDataBuf in this TU. Mickey's corresponding range remains inside the
 * anonymous whole-program BSS, so the buffer is declared extern and fixed at
 * 0x800D7E20 by symbol_addrs.us.txt; no C object claims its storage.
 *
 * PROVENANCE: the bodies and private SDK name are adapted from Jet Force
 * Gemini's `libultra/src/io/motor.c`, a permitted published decomp source
 * under docs/CLEANROOM.md. Mickey's bytes decide the local layout and flags.
 */

#include "PR/os_internal.h"
#include "PR/os_version.h"
#include "PRinternal/controller.h"
#include "PRinternal/macros.h"
#include "PRinternal/siint.h"

#define MOTOR_START 1
#define MOTOR_STOP  0

#define READFORMAT(ptr) ((__OSContRamReadFormat *)(ptr))

extern OSPifRam __MotorDataBuf[MAXCONTROLLERS];

s32 __osMotorAccess(OSPfs *pfs, s32 flag);

s32 osMotorStop(OSPfs *pfs) {
    return __osMotorAccess(pfs, MOTOR_STOP);
}

s32 osMotorStart(OSPfs *pfs) {
    return __osMotorAccess(pfs, MOTOR_START);
}

s32 __osMotorAccess(OSPfs *pfs, s32 flag) {
    int i;
    s32 ret;
    u8 *ptr = (u8 *)&__MotorDataBuf[pfs->channel];

    if (!(pfs->status & PFS_MOTOR_INITIALIZED)) {
        return PFS_ERR_INVALID;
    }

    __osSiGetAccess();
    __MotorDataBuf[pfs->channel].pifstatus = CONT_CMD_EXE;
    ptr += pfs->channel;

    for (i = 0; i < BLOCKSIZE; i++) {
        READFORMAT(ptr)->data[i] = flag;
    }

    __osContLastCmd = CONT_CMD_END;
    __osSiRawStartDma(OS_WRITE, &__MotorDataBuf[pfs->channel]);
    osRecvMesg(pfs->queue, NULL, OS_MESG_BLOCK);
    ret = __osSiRawStartDma(OS_READ, &__MotorDataBuf[pfs->channel]);
    osRecvMesg(pfs->queue, NULL, OS_MESG_BLOCK);

    ret = READFORMAT(ptr)->rxsize & CHNL_ERR_MASK;
    if (!ret) {
        if (!flag) {
            if (READFORMAT(ptr)->datacrc != 0) {
                ret = PFS_ERR_CONTRFAIL;
            }
        } else {
            if (READFORMAT(ptr)->datacrc != 0xEB) {
                ret = PFS_ERR_CONTRFAIL;
            }
        }
    }

    __osSiRelAccess();
    return ret;
}

static void __osMakeMotorData(int channel, OSPifRam *mdata) {
    u8 *ptr = (u8 *)mdata->ramarray;
    __OSContRamReadFormat ramreadformat;
    int i;

    ramreadformat.dummy = CONT_CMD_NOP;
    ramreadformat.txsize = CONT_CMD_WRITE_PAK_TX;
    ramreadformat.rxsize = CONT_CMD_WRITE_PAK_RX;
    ramreadformat.cmd = CONT_CMD_WRITE_PAK;
    ramreadformat.addrh = CONT_BLOCK_RUMBLE >> 3;
    ramreadformat.addrl = (u8)(__osContAddressCrc(CONT_BLOCK_RUMBLE) |
                              (CONT_BLOCK_RUMBLE << 5));

    if (channel != 0) {
        for (i = 0; i < channel; i++) {
            *ptr++ = CONT_CMD_REQUEST_STATUS;
        }
    }

    *READFORMAT(ptr) = ramreadformat;
    ptr += sizeof(__OSContRamReadFormat);
    ptr[0] = CONT_CMD_END;
}

s32 osMotorInit(OSMesgQueue *mq, OSPfs *pfs, int channel) {
    s32 ret;
    u8 temp[BLOCKSIZE];

    pfs->queue = mq;
    pfs->channel = channel;
    pfs->activebank = 0xFF;
    pfs->status = 0;

    ret = SELECT_BANK(pfs, 0xFE);

    if (ret == PFS_ERR_NEW_PACK) {
        ret = SELECT_BANK(pfs, 0x80);
    }

    if (ret != 0) {
        return ret;
    }

    ret = __osContRamRead(mq, channel, CONT_BLOCK_DETECT, temp);

    if (ret == PFS_ERR_NEW_PACK) {
        ret = PFS_ERR_CONTRFAIL;
    }

    if (ret != 0) {
        return ret;
    } else if (temp[31] == 0xFE) {
        return PFS_ERR_DEVICE;
    }

    ret = SELECT_BANK(pfs, 0x80);
    if (ret == PFS_ERR_NEW_PACK) {
        ret = PFS_ERR_CONTRFAIL;
    }

    if (ret != 0) {
        return ret;
    }

    ret = __osContRamRead(mq, channel, CONT_BLOCK_DETECT, temp);
    if (ret == PFS_ERR_NEW_PACK) {
        ret = PFS_ERR_CONTRFAIL;
    }

    if (ret != 0) {
        return ret;
    } else if (temp[31] != 0x80) {
        return PFS_ERR_DEVICE;
    }

    if (!(pfs->status & PFS_MOTOR_INITIALIZED)) {
        __osMakeMotorData(channel, &__MotorDataBuf[channel]);
    }

    pfs->status = PFS_MOTOR_INITIALIZED;
    return 0;
}
