#ifndef _SPTASK_H_
#define _SPTASK_H_

#include "PR/ultratypes.h"

typedef struct {
    u32 type;
    u32 flags;
    u64 *ucode_boot;
    u32 ucode_boot_size;
    u64 *ucode;
    u32 ucode_size;
    u64 *ucode_data;
    u32 ucode_data_size;
    u64 *dram_stack;
    u32 dram_stack_size;
    u64 *output_buff;
    u64 *output_buff_size;
    u64 *data_ptr;
    u32 data_size;
    u64 *yield_data_ptr;
    u32 yield_data_size;
} OSTask_t;

typedef union {
    OSTask_t t;
    s64 force_structure_alignment;
} OSTask;

typedef u32 OSYieldResult;

#define OS_TASK_YIELDED 0x0001
#define OS_TASK_DP_WAIT 0x0002
#define OS_TASK_LOADABLE 0x0004
#define OS_YIELD_DATA_SIZE 0x900

#endif
