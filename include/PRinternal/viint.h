#ifndef _VIINT_H_
#define _VIINT_H_

#include "PR/os_internal.h"
#include "PR/os_vi.h"

#define VI_STATE_MODE_UPDATED   0x01
#define VI_STATE_XSCALE_UPDATED 0x02
#define VI_STATE_YSCALE_UPDATED 0x04
#define VI_STATE_CTRL_UPDATED   0x08
#define VI_STATE_BUFFER_UPDATED 0x10
#define VI_STATE_BLACK          0x20
#define VI_STATE_REPEATLINE     0x40
#define VI_STATE_FADE           0x80

#define VI_SCALE_MASK       0xFFF
#define VI_2_10_FPART_MASK  0x3FF
#define VI_SUBPIXEL_SH      0x10

typedef struct {
    f32 factor;
    u16 offset;
    u16 pad;
    u32 scale;
} __OSViScale;

typedef struct {
    u16 state;
    u16 retraceCount;
    void *framep;
    OSViMode *modep;
    u32 control;
    OSMesgQueue *msgq;
    OSMesg msg;
    __OSViScale x;
    __OSViScale y;
} __OSViContext;

extern __OSViContext *__osViCurr;
extern __OSViContext *__osViNext;
extern u32 __additional_scanline;
extern u32 __osViIntrCount;

__OSViContext *__osViGetCurrentContext(void);
void __osViInit(void);
void __osViSwapContext(void);

#endif
