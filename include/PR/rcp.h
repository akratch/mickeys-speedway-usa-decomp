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
#define K1_TO_PHYS(x) ((u32)(x) & 0x1FFFFFFF)

#define IO_READ(addr)         (*(vu32 *)PHYS_TO_K1(addr))
#define IO_WRITE(addr, data)  (*(vu32 *)PHYS_TO_K1(addr) = (u32)(data))

#define PIF_RAM_START 0x1FC007C0
#define PIF_RAM_END   0x1FC007FF

/* Signal processor (RSP) */
#define SP_BASE_REG    0x04040000
#define SP_IMEM_START  0x04001000
#define SP_MEM_ADDR_REG  (SP_BASE_REG + 0x00)
#define SP_DRAM_ADDR_REG (SP_BASE_REG + 0x04)
#define SP_RD_LEN_REG    (SP_BASE_REG + 0x08)
#define SP_WR_LEN_REG    (SP_BASE_REG + 0x0C)
#define SP_STATUS_REG  (SP_BASE_REG + 0x10)
#define SP_PC_REG      0x04080000

#define SP_STATUS_HALT       0x0001
#define SP_STATUS_BROKE      0x0002
#define SP_STATUS_DMA_BUSY   0x0004
#define SP_STATUS_DMA_FULL   0x0008
#define SP_STATUS_IO_FULL    0x0010
#define SP_STATUS_SIG0       0x0080
#define SP_STATUS_SIG1       0x0100
#define SP_STATUS_SIG2       0x0200
#define SP_STATUS_YIELD      SP_STATUS_SIG0
#define SP_STATUS_YIELDED    SP_STATUS_SIG1
#define SP_STATUS_TASKDONE   SP_STATUS_SIG2

/* Writes to SP_STATUS_REG use a different, set/clear-paired bit layout from
   reads. Only the bits matched code writes are listed. */
#define SP_CLR_HALT       (1 << 0)
#define SP_CLR_BROKE      (1 << 2)
#define SP_CLR_SSTEP      (1 << 5)
#define SP_SET_INTR_BREAK (1 << 8)
#define SP_CLR_SIG0       (1 << 9)
#define SP_SET_SIG0       (1 << 10)
#define SP_CLR_SIG1       (1 << 11)
#define SP_CLR_SIG2       (1 << 13)
#define SP_CLR_YIELD      SP_CLR_SIG0
#define SP_CLR_YIELDED    SP_CLR_SIG1
#define SP_CLR_TASKDONE   SP_CLR_SIG2

/* Display processor command interface (RDP) */
#define DPC_BASE_REG    0x04100000
#define DPC_START_REG   (DPC_BASE_REG + 0x00)
#define DPC_END_REG     (DPC_BASE_REG + 0x04)
#define DPC_STATUS_REG  (DPC_BASE_REG + 0x0C)

#define DPC_CLR_XBUS_DMEM_DMA    (1 << 0)
#define DPC_STATUS_XBUS_DMEM_DMA (1 << 0)
#define DPC_STATUS_DMA_BUSY  0x0100

/* Audio interface */
#define AI_BASE_REG    0x04500000
#define AI_DRAM_ADDR_REG (AI_BASE_REG + 0x00)
#define AI_LEN_REG     (AI_BASE_REG + 0x04)
#define AI_CONTROL_REG (AI_BASE_REG + 0x08)
#define AI_STATUS_REG  (AI_BASE_REG + 0x0C)
#define AI_DACRATE_REG (AI_BASE_REG + 0x10)
#define AI_BITRATE_REG (AI_BASE_REG + 0x14)

#define AI_CONTROL_DMA_ON   1
#define AI_STATUS_FIFO_FULL  0x80000000
#define AI_MIN_DAC_RATE     132
#define AI_MAX_DAC_RATE     16384
#define AI_MAX_BIT_RATE     16

/* Video interface */
#define VI_BASE_REG      0x04400000
#define VI_CONTROL_REG  (VI_BASE_REG + 0x00)
#define VI_ORIGIN_REG   (VI_BASE_REG + 0x04)
#define VI_WIDTH_REG    (VI_BASE_REG + 0x08)
#define VI_INTR_REG     (VI_BASE_REG + 0x0C)
#define VI_CURRENT_REG  (VI_BASE_REG + 0x10)
#define VI_BURST_REG    (VI_BASE_REG + 0x14)
#define VI_V_SYNC_REG   (VI_BASE_REG + 0x18)
#define VI_H_SYNC_REG   (VI_BASE_REG + 0x1C)
#define VI_LEAP_REG     (VI_BASE_REG + 0x20)
#define VI_H_START_REG  (VI_BASE_REG + 0x24)
#define VI_V_START_REG  (VI_BASE_REG + 0x28)
#define VI_V_BURST_REG  (VI_BASE_REG + 0x2C)
#define VI_X_SCALE_REG  (VI_BASE_REG + 0x30)
#define VI_Y_SCALE_REG  (VI_BASE_REG + 0x34)

#define VI_NTSC_CLOCK 48681812
#define VI_PAL_CLOCK  49656530
#define VI_MPAL_CLOCK 48628316

#define VI_CTRL_GAMMA_DITHER_ON  0x00004
#define VI_CTRL_GAMMA_ON         0x00008
#define VI_CTRL_DIVOT_ON         0x00010
#define VI_CTRL_ANTIALIAS_MASK   0x00300
#define VI_CTRL_DITHER_FILTER_ON 0x10000

/* Peripheral interface */
#define PI_BASE_REG          0x04600000
#define PI_DRAM_ADDR_REG     (PI_BASE_REG + 0x00)
#define PI_CART_ADDR_REG     (PI_BASE_REG + 0x04)
#define PI_RD_LEN_REG        (PI_BASE_REG + 0x08)
#define PI_WR_LEN_REG        (PI_BASE_REG + 0x0C)
#define PI_STATUS_REG        (PI_BASE_REG + 0x10)
#define PI_BSD_DOM1_LAT_REG  (PI_BASE_REG + 0x14)
#define PI_BSD_DOM1_PWD_REG  (PI_BASE_REG + 0x18)
#define PI_BSD_DOM1_PGS_REG  (PI_BASE_REG + 0x1C)
#define PI_BSD_DOM1_RLS_REG  (PI_BASE_REG + 0x20)
#define PI_BSD_DOM2_LAT_REG  (PI_BASE_REG + 0x24)
#define PI_BSD_DOM2_PWD_REG  (PI_BASE_REG + 0x28)
#define PI_BSD_DOM2_PGS_REG  (PI_BASE_REG + 0x2C)
#define PI_BSD_DOM2_RLS_REG  (PI_BASE_REG + 0x30)

#define PI_STATUS_DMA_BUSY  0x01
#define PI_STATUS_IO_BUSY   0x02
#define PI_STATUS_CLR_INTR  0x02
#define PI_CLR_INTR         PI_STATUS_CLR_INTR

#define DEVICE_TYPE_INIT 7
#define DEVICE_TYPE_64DD 2

/* Serial interface */
#define SI_BASE_REG    0x04800000
#define SI_DRAM_ADDR_REG      (SI_BASE_REG + 0x00)
#define SI_PIF_ADDR_RD64B_REG (SI_BASE_REG + 0x04)
#define SI_PIF_ADDR_WR64B_REG (SI_BASE_REG + 0x10)
#define SI_STATUS_REG  (SI_BASE_REG + 0x18)

#define SI_STATUS_DMA_BUSY  0x0001
#define SI_STATUS_RD_BUSY   0x0002

#endif /* _RCP_H_ */
