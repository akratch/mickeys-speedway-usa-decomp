/*
 * libultra: build and parse the PIF-RAM request block for the four
 * controller channels.
 *
 * ROM 0x6F8E8-0x6F9B8 (__osContGetInitData) and 0x6F9B8-0x6FAAC
 * (__osPackRequestData), VRAM 0x8006F8E8/0x8006F9B8. `osContInit` (ROM
 * 0x6F6F0) stays `#pragma GLOBAL_ASM`: it needs `osClockRate`, still
 * asm in `initialize.c`.
 *
 * Flags: -O1 -mips2 -32, the corridor default (docs/modules.md section 6.1).
 *
 * DEVIATION FROM THE REFERENCE: the SDK source defines __osContPifRam,
 * __osContLastCmd and __osMaxControllers in this translation unit. Here
 * they are `extern`: Mickey's .data and .bss are still one unsplit
 * subsegment (docs/modules.md section 6.3), so no C file can own an
 * object yet -- see the piacs.c precedent in this directory. The emitted
 * .text is the same either way: every reference is a %hi/%lo pair on a
 * symbol the linker resolves, and symbol_addrs.us.txt now gives
 * __osContPifRam its own fixed address (0x800D80F0, 0x40 bytes, landing
 * exactly on __osContLastCmd with no slack) alongside the pak-side
 * addresses that were already there.
 *
 * PROVENANCE: the body is N64 SDK libultra source as published in public
 * decomp trees (JFG's among them), a permitted source under
 * docs/CLEANROOM.md; see docs/modules.md section 1.3.
 */

#include "PR/os_internal.h"
#include "PRinternal/controller.h"
#include "PRinternal/macros.h"

#pragma GLOBAL_ASM("asm/nonmatchings/libultra/controller/osContInit.s")

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
