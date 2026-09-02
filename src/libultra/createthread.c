#include "PR/os_internal.h"
#include "PR/R4300.h"
#include "PR/ultraerror.h"
#include "PRinternal/osint.h"

typedef union {
    struct {
        f32 f_odd;
        f32 f_even;
    } f;
    f64 d;
} MickeyOSfp;

typedef struct {
    u64 at, v0, v1, a0, a1, a2, a3;
    u64 t0, t1, t2, t3, t4, t5, t6, t7;
    u64 s0, s1, s2, s3, s4, s5, s6, s7;
    u64 t8, t9;
    u64 gp, sp, s8, ra;
    u64 lo, hi;
    u32 sr, pc, cause, badvaddr, rcp;
    u32 fpcsr;
    MickeyOSfp fp0, fp2, fp4, fp6, fp8, fp10, fp12, fp14;
    MickeyOSfp fp16, fp18, fp20, fp22, fp24, fp26, fp28, fp30;
} MickeyOSThreadContext;

typedef struct MickeyOSThread_s {
    struct MickeyOSThread_s *next;
    OSPri priority;
    struct MickeyOSThread_s **queue;
    struct MickeyOSThread_s *tlnext;
    u16 state;
    u16 flags;
    OSId id;
    s32 fp;
    void *thprof;
    MickeyOSThreadContext context;
} MickeyOSThread;

extern void __osCleanupThread(void);

/* PROVENANCE: body adapted from Jet Force Gemini's permitted libultra source,
 * libultra/src/os/createthread.c; Mickey's local OSThread header exposes only
 * its fixed prefix, so this TU supplies the same SDK context layout locally. */
void osCreateThread(OSThread *t, OSId id, void (*entry)(void *), void *arg,
                    void *sp, OSPri priority) {
    register u32 saveMask;
    OSIntMask mask;

#define T ((MickeyOSThread *)t)

#ifdef _DEBUG
    if ((u32)sp & 0x7) {
        __osError(ERR_OSCREATETHREAD_SP, 1, sp);
        return;
    }

    if ((priority < OS_PRIORITY_IDLE) || (priority > OS_PRIORITY_MAX)) {
        __osError(ERR_OSCREATETHREAD_PRI, 1, priority);
        return;
    }
#endif

    T->id = id;
    T->priority = priority;
    T->next = NULL;
    T->queue = NULL;
    T->context.pc = (u32)entry;
    T->context.a0 = (s64)(s32)arg;
    T->context.sp = (s64)(s32)sp - 16;
    T->context.ra = (s64)(s32)__osCleanupThread;
    mask = OS_IM_ALL;
    T->context.sr = (0x04000000 | (0x0000FF00 | 0x00000001)) | 0x00000002;
    T->context.rcp = (mask & 0x003F0000) >> 16;
    T->context.fpcsr = FPCSR_FS | FPCSR_EV | FPCSR_RM_RN;
    T->fp = 0;
    T->state = OS_STATE_STOPPED;
    T->flags = 0;

    if (id < 0) {
        T->id = -id;
        T->context.sr &= ~0x04000000;
    }

    saveMask = __osDisableInt();
    T->tlnext = (struct MickeyOSThread_s *)__osActiveQueue;
    __osActiveQueue = t;
    __osRestoreInt(saveMask);
}

#undef T
