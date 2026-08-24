#ifndef _R4300_H_
#define _R4300_H_

#include "PR/ultratypes.h"

#define K0BASE 0x80000000
#define K1BASE 0xA0000000
#define K2BASE 0xC0000000

#define K0_TO_PHYS(x) ((u32)(x) & 0x1FFFFFFF)
#define K1_TO_PHYS(x) ((u32)(x) & 0x1FFFFFFF)
#define IS_KSEG0(x) ((u32)(x) >= K0BASE && (u32)(x) < K1BASE)
#define IS_KSEG1(x) ((u32)(x) >= K1BASE && (u32)(x) < K2BASE)

#endif
