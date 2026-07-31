#ifndef _RCP_H_
#define _RCP_H_

/*
 * RCP memory-mapped register addresses and the status bits matched code reads.
 *
 * These are hardware facts about the Nintendo 64's RCP, not anything derived
 * from a particular SDK: the physical addresses and bit assignments are
 * documented in every public N64 hardware reference. Written from scratch for
 * this project, and deliberately minimal -- only registers and bits that
 * matched translation units actually reference are listed. Extend as needed.
 *
 * Registers are always reached through KSEG1 (uncached, unmapped) so that a
 * read really goes to the device instead of the data cache.
 */

#include "PR/ultratypes.h"

#define PHYS_TO_K1(x) ((u32)(x) | 0xA0000000)

#define IO_READ(addr)         (*(vu32 *)PHYS_TO_K1(addr))
#define IO_WRITE(addr, data)  (*(vu32 *)PHYS_TO_K1(addr) = (u32)(data))

/* Signal processor (RSP) */
#define SP_BASE_REG    0x04040000
#define SP_STATUS_REG  (SP_BASE_REG + 0x10)

#define SP_STATUS_HALT       0x0001
#define SP_STATUS_BROKE      0x0002
#define SP_STATUS_DMA_BUSY   0x0004
#define SP_STATUS_DMA_FULL   0x0008
#define SP_STATUS_IO_FULL    0x0010

/* Writes to SP_STATUS_REG use a different, set/clear-paired bit layout from
   reads. Only the bits matched code writes are listed. */
#define SP_SET_SIG0  0x0400   /* the yield-requested signal */

/* Display processor command interface (RDP) */
#define DPC_BASE_REG    0x04100000
#define DPC_STATUS_REG  (DPC_BASE_REG + 0x0C)

#define DPC_STATUS_DMA_BUSY  0x0100

/* Audio interface */
#define AI_BASE_REG    0x04500000
#define AI_LEN_REG     (AI_BASE_REG + 0x04)
#define AI_STATUS_REG  (AI_BASE_REG + 0x0C)

#define AI_STATUS_FIFO_FULL  0x80000000

/* Serial interface */
#define SI_BASE_REG    0x04800000
#define SI_STATUS_REG  (SI_BASE_REG + 0x18)

#define SI_STATUS_DMA_BUSY  0x0001
#define SI_STATUS_RD_BUSY   0x0002

#endif /* _RCP_H_ */
