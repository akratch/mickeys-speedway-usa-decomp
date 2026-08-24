/*
 * libultra: build and parse the PIF-RAM request block for the four
 * controller channels.
 *
 * The complete 0x3C0-byte text object and its 0x10-byte initialized-data
 * section are byte-identical to Jet Force Gemini's built
 * `libultra/src/io/controller.c.o` after relocation.
 *
 * Flags: -O1 -mips2 -32, the corridor default (docs/modules.md section 6.1).
 *
 * DEVIATION FROM THE REFERENCE: the SDK source defines its 0x90-byte BSS in
 * this TU. Mickey's BSS remains an anonymous gap, so those objects are
 * declared extern and fixed by symbol_addrs.us.txt instead; no C object
 * claims their storage. The explicitly initialized __osContinitialized
 * remains a real C definition because the matching .data slice is carved.
 *
 * PROVENANCE: the body is N64 SDK libultra source as published in public
 * decomp trees (JFG's among them), a permitted source under
 * docs/CLEANROOM.md; see docs/modules.md section 1.3.
 */

#include "PR/os_internal.h"
#include "PRinternal/controller.h"
#include "PRinternal/macros.h"
#include "PRinternal/osint.h"
#include "PRinternal/siint.h"

#define HALF_MIL_CYCLES 500000U
#define ONE_MIL_CYCLES  1000000U
#define HALF_A_SECOND   (HALF_MIL_CYCLES * osClockRate / ONE_MIL_CYCLES)

s32 __osContinitialized = FALSE;

s32 osContInit(OSMesgQueue *mq, u8 *bitpattern, OSContStatus *data) {
    OSMesg dummy;
    s32 ret = 0;
    OSTime t;
    OSTimer mytimer;
    OSMesgQueue timerMesgQueue;

    if (__osContinitialized) {
        return 0;
    }

    __osContinitialized = TRUE;

    t = osGetTime();
    if (HALF_A_SECOND > t) {
        osCreateMesgQueue(&timerMesgQueue, &dummy, 1);
        osSetTimer(&mytimer, HALF_A_SECOND - t, 0, &timerMesgQueue, &dummy);
        osRecvMesg(&timerMesgQueue, &dummy, OS_MESG_BLOCK);
    }

    __osMaxControllers = MAXCONTROLLERS;
    __osPackRequestData(CONT_CMD_REQUEST_STATUS);

    ret = __osSiRawStartDma(OS_WRITE, __osContPifRam.ramarray);
    osRecvMesg(mq, &dummy, OS_MESG_BLOCK);

    ret = __osSiRawStartDma(OS_READ, __osContPifRam.ramarray);
    osRecvMesg(mq, &dummy, OS_MESG_BLOCK);

    __osContGetInitData(bitpattern, data);
    __osContLastCmd = CONT_CMD_REQUEST_STATUS;
    __osSiCreateAccessQueue();
    osCreateMesgQueue(&__osEepromTimerQ, &__osEepromTimerMsg, 1);

    return ret;
}

void __osContGetInitData(u8 *pattern, OSContStatus *data) {
    u8 *ptr;
    __OSContRequesFormat requestHeader;
    s32 i;
    u8 bits = 0;

    ptr = (u8 *)__osContPifRam.ramarray;
    for (i = 0; i < __osMaxControllers; i++, ptr += sizeof(requestHeader), data++) {
        requestHeader = *(__OSContRequesFormat *)ptr;
        data->errno = CHNL_ERR(requestHeader);

        if (data->errno != 0) {
            continue;
        }

        data->type = requestHeader.typel << 8 | requestHeader.typeh;
        data->status = requestHeader.status;
        bits |= 1 << i;
    }
    *pattern = bits;
}

void __osPackRequestData(u8 cmd) {
    u8 *ptr;
    __OSContRequesFormat requestHeader;
    s32 i;

    /* ROM matches the RAREDIFFS `<=` bound (JFG's controller.c): one extra
     * iteration zeroes pifstatus too, since it is the array's next word. */
    for (i = 0; i <= ARRLEN(__osContPifRam.ramarray); i++) {
        __osContPifRam.ramarray[i] = 0;
    }

    __osContPifRam.pifstatus = CONT_CMD_EXE;
    ptr = (u8 *)__osContPifRam.ramarray;
    requestHeader.dummy = CONT_CMD_NOP;
    requestHeader.txsize = CONT_CMD_RESET_TX;
    requestHeader.rxsize = CONT_CMD_RESET_RX;
    requestHeader.cmd = cmd;
    requestHeader.typeh = CONT_CMD_NOP;
    requestHeader.typel = CONT_CMD_NOP;
    requestHeader.status = CONT_CMD_NOP;
    requestHeader.dummy1 = CONT_CMD_NOP;

    for (i = 0; i < __osMaxControllers; i++) {
        *(__OSContRequesFormat *)ptr = requestHeader;
        ptr += sizeof(requestHeader);
    }
    *ptr = CONT_CMD_END;
}
