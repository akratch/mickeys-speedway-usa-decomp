/*
 * Resident gzip front end -- ROM 0x4E1E0-0x4EA60.
 *
 * DKR's published src/gzip.c identifies this source unit and supplies the
 * byteswap32 body below. Mickey's bytes remain decisive: the remaining two
 * functions retain their generated assembly until separately reconstructed.
 */

#include "PR/ultratypes.h"
#include "PR/os_internal.h"
#include "PR/os_message.h"
#include "game/pi.h"
#include "PRinternal/piint.h"

extern s32 D_8007D630;
extern u8 *D_8007D634;
extern u8 *D_8007D638;
extern u8 D_800D6A90[];
extern s32 D_800D6AA0;
extern s32 D_800D6AA4;
extern void *func_8002B280(s32 size, s32 tag);
extern s32 gzip_inflate_block(void);
extern OSThread D_800D6740;
extern OSMesgQueue D_800D6A70;
extern OSMesg D_800D6A88;
extern void __osDevMgrMain(void *);

/*
 * PROVENANCE: adapted from the published Diddy Kong Racing libultra
 * `osCreatePiManager` source. Mickey's fixed BSS objects and instruction
 * stream remain authoritative; this body is compiled and independently
 * compared against Mickey's function.
 */
void func_8004D5E0(OSPri priority, OSMesgQueue *queue, OSMesg *messages,
                   s32 count) {
    u32 savedMask;
    OSPri oldPri;
    OSPri myPri;

    if (__osPiDevMgr.active) {
        return;
    }
    osCreateMesgQueue(queue, messages, count);
    osCreateMesgQueue(&D_800D6A70, &D_800D6A88, 1);
    if (!__osPiAccessQueueEnabled) {
        __osPiCreateAccessQueue();
    }
    osSetEventMesg(8, &D_800D6A70, (OSMesg)0x22222222);
    oldPri = -1;
    myPri = osGetThreadPri(NULL);
    if (myPri < priority) {
        oldPri = myPri;
        osSetThreadPri(NULL, priority);
    }
    savedMask = __osDisableInt();
    __osPiDevMgr.active = 1;
    __osPiDevMgr.thread = &D_800D6740;
    __osPiDevMgr.cmdQueue = queue;
    __osPiDevMgr.evtQueue = &D_800D6A70;
    __osPiDevMgr.acsQueue = &__osPiAccessQueue;
    __osPiDevMgr.dma = __osPiRawStartDma;
    __osPiDevMgr.edma = __osEPiRawStartDma;
    osCreateThread(&D_800D6740, 0, __osDevMgrMain, &__osPiDevMgr,
                   &D_800D6A70, priority);
    osStartThread(&D_800D6740);
    __osRestoreInt(savedMask);
    if (oldPri != -1) {
        osSetThreadPri(NULL, oldPri);
    }
}

void func_8004D750(void) {
    D_8007D630 = (s32) func_8002B280(0x2800, 0x8F);
}

/*
 * PROVENANCE: adapted from Diddy Kong Racing's published src/gzip.c.
 * Mickey's compiled and linked function is independently byte-identical.
 */
s32 byteswap32(u8 *arg0) {
    s32 value;

    value = *arg0++;
    value |= *arg0++ << 8;
    value |= *arg0++ << 16;
    value |= *arg0 << 24;
    return value;
}

s32 func_8004D7A8(s32 assetIndex, s32 assetOffset) {
    piRomLoadSection(assetIndex, (u32) D_800D6A90, assetOffset, 8);
    return byteswap32(D_800D6A90);
}

u8 *func_8004D7E0(u8 *compressed, u8 *output) {
    D_8007D634 = compressed + 5;
    D_8007D638 = output;
    D_800D6AA4 = 0;
    D_800D6AA0 = 0;
    if (gzip_inflate_block() != 0) {
        do {
        } while (gzip_inflate_block() != 0);
    }
    return output;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/gzip/func_8004D840.s")
