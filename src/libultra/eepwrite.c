/*
 * Mickey's EEPROM write TU retains the official SDK packet construction and
 * SI transaction, extended for the four EEPROM type encodings present in the
 * game. Placeholder names preserve the authenticated caller relocations.
 *
 * PROVENANCE: adapted from official libultra source published in the Banjo-
 * Kazooie decomp at `lib/ultralib/src/io/conteepwrite.c`, pinned in
 * `tools/reference-builds.lock`. This is a permitted published decomp source
 * under docs/CLEANROOM.md. The additional type cases and limits are derived
 * from Mickey's own ROM bytes.
 */

#include "PR/os_internal.h"
#include "PR/rcp.h"
#include "PRinternal/controller.h"
#include "PRinternal/siint.h"

#define MICKEY_CONT_EEPROM_TYPE_MASK 0xF000
#define MICKEY_CONT_EEPROM_4K 0x8000
#define MICKEY_CONT_EEPROM_16K 0xC000
#define MICKEY_CONT_EEPROM_128K 0xA000
#define MICKEY_CONT_EEPROM_512K 0x9000
#define MICKEY_CONT_EEPROM_BUSY 0x80
#define MICKEY_CONT_NO_RESPONSE_ERROR 8

typedef struct {
    u8 txsize;
    u8 rxsize;
    u8 cmd;
    u8 address;
    u8 data[8];
} MickeyEepromFormat;

extern OSPifRam D_800D8710;
extern s32 D_800D874C;
extern s32 __osEepStatus(OSMesgQueue *mq, OSContStatus *data);

void func_800739A4(u8 address, u8 *buffer);

s32 func_800737A0(OSMesgQueue *mq, u8 address, u8 *buffer) {
    s32 ret = 0;
    u16 type;
    u8 *ptr = (u8 *) D_800D8710.ramarray;
    MickeyEepromFormat eepromformat;
    OSContStatus sdata;

    __osSiGetAccess();
    ret = __osEepStatus(mq, &sdata);
    type = sdata.type & MICKEY_CONT_EEPROM_TYPE_MASK;

    if (ret == 0) {
        switch (type) {
            case MICKEY_CONT_EEPROM_4K:
                if (address >= 0x40) {
                    ret = -1;
                }
                break;
            case MICKEY_CONT_EEPROM_16K:
                if (address >= 0x100) {
                    ret = -1;
                }
                break;
            case MICKEY_CONT_EEPROM_128K:
                if (address >= 0x1000) {
                    ret = -1;
                }
                break;
            case MICKEY_CONT_EEPROM_512K:
                if (address >= 0x4000) {
                    ret = -1;
                }
                break;
            default:
                ret = MICKEY_CONT_NO_RESPONSE_ERROR;
                break;
        }
    }

    if (ret != 0) {
        __osSiRelAccess();
        return ret;
    }

    while (sdata.status & MICKEY_CONT_EEPROM_BUSY) {
        __osEepStatus(mq, &sdata);
    }

    func_800739A4(address, buffer);
    ret = __osSiRawStartDma(OS_WRITE, &D_800D8710);
    osRecvMesg(mq, 0, OS_MESG_BLOCK);
    ret = __osSiRawStartDma(OS_READ, &D_800D8710);
    __osContLastCmd = CONT_CMD_WRITE_EEPROM;
    osRecvMesg(mq, 0, OS_MESG_BLOCK);

    ptr += MAXCONTROLLERS;
    eepromformat = *(MickeyEepromFormat *) ptr;
    ret = CHNL_ERR(eepromformat);

    __osSiRelAccess();
    return ret;
}

void func_800739A4(u8 address, u8 *buffer) {
    u8 *ptr = (u8 *) D_800D8710.ramarray;
    MickeyEepromFormat eepromformat;
    int i;

    D_800D874C = CONT_CMD_EXE;
    eepromformat.txsize = 10;
    eepromformat.rxsize = 1;
    eepromformat.cmd = CONT_CMD_WRITE_EEPROM;
    eepromformat.address = address;

    for (i = 0; i < 8; i++) {
        eepromformat.data[i] = *buffer++;
    }

    for (i = 0; i < MAXCONTROLLERS; i++) {
        *ptr++ = 0;
    }

    *(MickeyEepromFormat *) ptr = eepromformat;
    ptr += sizeof(MickeyEepromFormat);
    *ptr = CONT_CMD_END;
}
