/*
 * libultra EEPROM status -- ROM 0x74680-0x748B0.
 *
 * PROVENANCE: adapted from Diddy Kong Racing's published SDK
 * src/io/conteepwrite.c::__osEepStatus. Only this exact function tail is
 * owned here; Mickey's preceding functions differ from the donor TU.
 *
 * Flags: DKR's measured -O1 -mips2 -Wab,-r4300_mul -w preset, compiled
 * directly so IDO retains the donor's line schedule.
 */

#include "PR/os_internal.h"
#include "PR/rcp.h"
#include "PRinternal/controller.h"
#include "PRinternal/siint.h"

extern OSPifRam D_800D8710;

s32 __osEepStatus(OSMesgQueue *mq, OSContStatus *data) {
    s32 ret = 0;
    int i;
    u8 *ptr = (u8 *) D_800D8710.ramarray;
    __OSContRequesFormat requestformat;

    for (i = 0; i < 16; i++) {
        D_800D8710.ramarray[i] = 0;
    }

    D_800D8710.pifstatus = CONT_CMD_EXE;
    ptr = (u8 *) D_800D8710.ramarray;

    for (i = 0; i < MAXCONTROLLERS; i++) {
        *ptr++ = CONT_CMD_REQUEST_STATUS;
    }

    requestformat.dummy = CONT_CMD_NOP;
    requestformat.txsize = CONT_CMD_REQUEST_STATUS_TX;
    requestformat.rxsize = CONT_CMD_REQUEST_STATUS_RX;
    requestformat.cmd = CONT_CMD_REQUEST_STATUS;
    requestformat.typeh = CONT_CMD_NOP;
    requestformat.typel = CONT_CMD_NOP;
    requestformat.status = CONT_CMD_NOP;
    requestformat.dummy1 = CONT_CMD_NOP;
    *(__OSContRequesFormat *) ptr = requestformat;
    ptr += sizeof(__OSContRequesFormat);
    *ptr = CONT_CMD_END;

    ret = __osSiRawStartDma(OS_WRITE, &D_800D8710);
    osRecvMesg(mq, 0, OS_MESG_BLOCK);
    __osContLastCmd = CONT_CMD_WRITE_EEPROM;
    ret = __osSiRawStartDma(OS_READ, &D_800D8710);
    osRecvMesg(mq, 0, OS_MESG_BLOCK);

    if (ret != 0) {
        return ret;
    }

    ptr = (u8 *) &D_800D8710;

    for (i = 0; i < MAXCONTROLLERS; i++) {
        *ptr++ = 0;
    }

    requestformat = *(__OSContRequesFormat *) ptr;
    data->errno = CHNL_ERR(requestformat);
    data->type = (requestformat.typel << 8) | requestformat.typeh;
    data->status = requestformat.status;

    if (data->errno != 0) {
        return data->errno;
    }

    return 0;
}
