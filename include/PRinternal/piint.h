#ifndef _PIINT_H_
#define _PIINT_H_

/*
 * PI-manager internals shared by the io/ translation units in this corridor.
 *
 * EPI_SYNC's shape is read off Mickey's __osEPiRawReadIo (ROM 0x74F20): the
 * guard is `__osCurrentHandle[domain]->type != pihandle->type` (`lbu 0x4(v1)`
 * against `lbu 0x4(a0)` at ROM 0x74F64), and the tail copies the five domain
 * bytes into the current handle rather than replacing the table entry. That is
 * the 2.0J form of the macro; the earlier form compares and assigns the
 * pointer itself and does not match these bytes.
 *
 * PROVENANCE: the macros and prototypes follow the N64 SDK internal header as
 * published in public decomp trees (JFG's `include/PRinternal/piint.h`), a
 * permitted source under docs/CLEANROOM.md; see docs/modules.md section 1.3.
 */

#include "PR/ultratypes.h"
#include "PR/os_internal.h"
#include "PR/os_message.h"
#include "PR/os_pi.h"
#include "PR/rcp.h"

extern OSDevMgr __osPiDevMgr;
extern OSPiHandle *__osCurrentHandle[2];
extern OSMesgQueue __osPiAccessQueue;
extern u32 __osPiAccessQueueEnabled;

void __osPiCreateAccessQueue(void);
s32 __osPiRawStartDma(s32 direction, u32 devAddr, void *dramAddr, u32 size);
s32 __osEPiRawStartDma(OSPiHandle *pihandle, s32 direction, u32 devAddr,
                       void *dramAddr, u32 size);

#define WAIT_ON_IOBUSY(stat)                                    \
    {                                                           \
        stat = IO_READ(PI_STATUS_REG);                          \
        while (stat & (PI_STATUS_IO_BUSY | PI_STATUS_DMA_BUSY)) \
            stat = IO_READ(PI_STATUS_REG);                      \
    }                                                           \
    (void)0

#define UPDATE_REG(pihandle, reg, var) \
    if (cHandle->var != pihandle->var) \
    IO_WRITE(reg, pihandle->var)

#define EPI_SYNC(pihandle, stat, domain)                            \
                                                                    \
    WAIT_ON_IOBUSY(stat);                                           \
                                                                    \
    domain = pihandle->domain;                                      \
    if (__osCurrentHandle[domain]->type != pihandle->type) {        \
        OSPiHandle *cHandle = __osCurrentHandle[domain];            \
        if (domain == PI_DOMAIN1) {                                 \
            UPDATE_REG(pihandle, PI_BSD_DOM1_LAT_REG, latency);     \
            UPDATE_REG(pihandle, PI_BSD_DOM1_PGS_REG, pageSize);    \
            UPDATE_REG(pihandle, PI_BSD_DOM1_RLS_REG, relDuration); \
            UPDATE_REG(pihandle, PI_BSD_DOM1_PWD_REG, pulse);       \
        } else {                                                    \
            UPDATE_REG(pihandle, PI_BSD_DOM2_LAT_REG, latency);     \
            UPDATE_REG(pihandle, PI_BSD_DOM2_PGS_REG, pageSize);    \
            UPDATE_REG(pihandle, PI_BSD_DOM2_RLS_REG, relDuration); \
            UPDATE_REG(pihandle, PI_BSD_DOM2_PWD_REG, pulse);       \
        }                                                           \
        cHandle->type = pihandle->type;                             \
        cHandle->latency = pihandle->latency;                       \
        cHandle->pageSize = pihandle->pageSize;                     \
        cHandle->relDuration = pihandle->relDuration;               \
        cHandle->pulse = pihandle->pulse;                           \
    }                                                               \
    (void)0

#endif /* _PIINT_H_ */
