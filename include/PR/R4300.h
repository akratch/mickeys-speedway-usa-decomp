#ifndef _R4300_H_
#define _R4300_H_

#include "PR/ultratypes.h"

#define K0BASE 0x80000000
#define K1BASE 0xA0000000
#define K2BASE 0xC0000000

#define UT_VEC  K0BASE
#define XUT_VEC (K0BASE + 0x80)
#define ECC_VEC (K0BASE + 0x100)
#define E_VEC   (K0BASE + 0x180)

#define SR_CU1 0x20000000

#define CAUSE_IP5 0x00001000

#define FPCSR_FS    0x01000000
#define FPCSR_EV    0x00000800
#define FPCSR_RM_RN 0x00000000

#define K0_TO_PHYS(x) ((u32)(x) & 0x1FFFFFFF)
#define K1_TO_PHYS(x) ((u32)(x) & 0x1FFFFFFF)
#define IS_KSEG0(x) ((u32)(x) >= K0BASE && (u32)(x) < K1BASE)
#define IS_KSEG1(x) ((u32)(x) >= K1BASE && (u32)(x) < K2BASE)

#endif
