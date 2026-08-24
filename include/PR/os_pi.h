#ifndef _OS_PI_H_
#define _OS_PI_H_

/*
 * Peripheral-interface types.
 *
 * Every offset below is read off Mickey's own instructions:
 *
 *   OSPiHandle   `sw t6,0x0(a1)` / `sw a1,0x0(v1)` at ROM 0x73078 links
 *                `next` at 0x00; __osEPiRawReadIo (ROM 0x74F20) loads
 *                `domain` with `lbu 0x9(a0)`, compares `type` at 0x4 and
 *                copies latency/pageSize/relDuration/pulse from 0x5..0x8,
 *                then reads `baseAddress` with `lw 0xC(a0)`.
 *   OSIoMesg     osEPiStartDma (ROM 0x73140) stores `piHandle` at 0x14,
 *                `hdr.type` with `sh` at 0x00 and reads `hdr.pri` with
 *                `lbu 0x2(a1)`; osPiStartDma (ROM 0x6FB90) writes
 *                dramAddr/devAddr/size at 0x8/0xC/0x10.
 *   OSDevMgr     `__osPiDevMgr` at 0x8007D600 is 0x1C bytes; osPiGetCmdQueue
 *                (ROM 0x73AD0) reads `active` at 0x0 and `cmdQueue` at 0x8.
 *
 * PROVENANCE: names follow the N64 SDK header as published in public decomp
 * trees (JFG's `include/PR/os_pi.h`), a permitted source under
 * docs/CLEANROOM.md; see docs/modules.md section 1.3.
 */

#include "PR/ultratypes.h"
#include "PR/os_message.h"

typedef struct {
    u32 errStatus;
    void *dramAddr;
    void *C2Addr;
    u32 sectorSize;
    u32 C1ErrNum;
    u32 C1ErrSector[4];
} __OSBlockInfo;

typedef struct {
    u32 cmdType;
    u16 transferMode;
    u16 blockNum;
    s32 sectorNum;
    u32 devAddr;
    u32 bmCtlShadow;
    u32 seqCtlShadow;
    __OSBlockInfo block[2];
} __OSTranxInfo;

typedef struct OSPiHandle_s {
    struct OSPiHandle_s *next;
    u8 type;
    u8 latency;
    u8 pageSize;
    u8 relDuration;
    u8 pulse;
    u8 domain;
    u32 baseAddress;
    u32 speed;
    __OSTranxInfo transferInfo;
} OSPiHandle;

typedef struct {
    u16 type;
    u8 pri;
    u8 status;
    OSMesgQueue *retQueue;
} OSIoMesgHdr;

typedef struct {
    OSIoMesgHdr hdr;
    void *dramAddr;
    u32 devAddr;
    u32 size;
    OSPiHandle *piHandle;
} OSIoMesg;

typedef struct {
    s32 active;
    OSThread *thread;
    OSMesgQueue *cmdQueue;
    OSMesgQueue *evtQueue;
    OSMesgQueue *acsQueue;
    s32 (*dma)(s32, u32, void *, u32);
    s32 (*edma)(OSPiHandle *, s32, u32, void *, u32);
} OSDevMgr;

/* Transfer direction. */
#ifndef OS_READ
#define OS_READ  0
#define OS_WRITE 1
#endif

/* I/O message types. The two osEPiStartDma writes -- 0xF and 0x10 at ROM
   0x73168 and 0x73174 -- fix the base at 10. */
#define OS_MESG_TYPE_BASE      10
#define OS_MESG_TYPE_LOOPBACK  (OS_MESG_TYPE_BASE + 0)
#define OS_MESG_TYPE_DMAREAD   (OS_MESG_TYPE_BASE + 1)
#define OS_MESG_TYPE_DMAWRITE  (OS_MESG_TYPE_BASE + 2)
#define OS_MESG_TYPE_VRETRACE  (OS_MESG_TYPE_BASE + 3)
#define OS_MESG_TYPE_COUNTER   (OS_MESG_TYPE_BASE + 4)
#define OS_MESG_TYPE_EDMAREAD  (OS_MESG_TYPE_BASE + 5)
#define OS_MESG_TYPE_EDMAWRITE (OS_MESG_TYPE_BASE + 6)

#define OS_MESG_PRI_NORMAL 0
#define OS_MESG_PRI_HIGH   1

#define PI_DOMAIN1 0
#define PI_DOMAIN2 1

extern OSPiHandle *__osPiTable;

OSMesgQueue *osPiGetCmdQueue(void);
s32 osPiStartDma(OSIoMesg *mb, s32 priority, s32 direction, u32 devAddr,
                 void *dramAddr, u32 size, OSMesgQueue *mq);
s32 osEPiStartDma(OSPiHandle *pihandle, OSIoMesg *mb, s32 direction);
s32 osEPiLinkHandle(OSPiHandle *EPiHandle);
s32 osEPiReadIo(OSPiHandle *pihandle, u32 devAddr, u32 *data);
s32 osEPiWriteIo(OSPiHandle *pihandle, u32 devAddr, u32 data);

#endif /* _OS_PI_H_ */
