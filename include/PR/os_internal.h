#ifndef _OS_INTERNAL_H_
#define _OS_INTERNAL_H_

/*
 * Prototypes for libultra-internal helpers that matched translation units in
 * this corridor define or call. Names come from byte-identity against a
 * reference build (see the provenance note in symbol_addrs.us.txt); the
 * signatures are read off the ROM's own calling convention.
 *
 * Minimal on purpose: add an entry when a translation unit needs it.
 */

#include "PR/ultratypes.h"
#include "PR/os_version.h"

typedef s32 OSPri;
typedef s32 OSId;
typedef u64 OSTime;

/* Scheduler prefix used by the matched thread/message routines. The offsets
 * are fixed by their loads and stores; the saved-register context follows but
 * is deliberately omitted until a C translation unit accesses it. */
typedef struct OSThread_s {
    struct OSThread_s *next;
    OSPri priority;
    struct OSThread_s **queue;
    struct OSThread_s *tlnext;
    u16 state;
    u16 flags;
    OSId id;
    s32 fp;
    void *thprof;
    u8 context[0x18C];
} OSThread;

#define OS_STATE_STOPPED  (1 << 0)
#define OS_STATE_RUNNABLE (1 << 1)
#define OS_STATE_RUNNING  (1 << 2)
#define OS_STATE_WAITING  (1 << 3)

#define OS_PRIORITY_MAX  255
#define OS_PRIORITY_IDLE 0

#ifndef OS_READ
#define OS_READ  0
#define OS_WRITE 1
#endif

OSThread *__osGetActiveQueue(void);

struct OSPiHandle_s;

void __osPiGetAccess(void);
void __osPiRelAccess(void);
s32 __osEPiRawReadIo(struct OSPiHandle_s *handle, u32 devAddr, u32 *data);
s32 __osEPiRawWriteIo(struct OSPiHandle_s *handle, u32 devAddr, u32 data);

/* Interrupt masking. `__osDisableInt` returns the previous mask, which every
   caller in this corridor hands straight back to `__osRestoreInt`. */
u32 __osDisableInt(void);
void __osRestoreInt(u32 mask);

/* The hardware-interrupt bits the global mask carries. Only the width is
   established here: __osSetGlobalIntMask (ROM 0x75080) ORs its argument into
   the word at 0x8008045C. */
typedef u32 OSHWIntr;
typedef u32 OSIntMask;

#define OS_IM_RCP 0x00000401
#define OS_IM_PI  0x00100401
#define OS_IM_ALL 0x003FFF01

#define OS_CLOCK_RATE 62500000LL
#define OS_APP_NMI_BUFSIZE 64

#define OS_TV_PAL  0
#define OS_TV_NTSC 1
#define OS_TV_MPAL 2

extern u32 __OSGlobalIntMask;
extern u32 __osShutdown;
extern OSTime osClockRate;

extern s32 osResetType;
extern s32 osTvType;
extern s32 osAppNMIBuffer[];

void __osSetGlobalIntMask(OSHWIntr mask);
void __osResetGlobalIntMask(OSHWIntr mask);

u32 __osGetSR(void);
void __osSetSR(u32 value);
u32 __osSetFpcCsr(u32 value);
u32 __osGetCause(void);
void osUnmapTLBAll(void);
void osMapTLBRdb(void);
void osInitialize(void);

void osStartThread(OSThread *thread);
void osCreateThread(OSThread *thread, OSId id, void (*entry)(void *),
                    void *arg, void *sp, OSPri priority);
void osStopThread(OSThread *thread);
void osDestroyThread(OSThread *thread);
void osYieldThread(void);
void osSetThreadPri(OSThread *thread, OSPri priority);
OSPri osGetThreadPri(OSThread *thread);

/* Set by the boot code at 0x80000308; `__osPiRawStartDma` (ROM 0x72850) ORs it
   with the device address before masking to a physical address. */
extern u32 osRomBase;

u32 osVirtualToPhysical(void *vaddr);

u32 osGetCount(void);

/* libc's hand-written block routines, still `asm`.
   The SDK's assembly weak-aliases each entry point to an underscored name and
   its header spells the plain one; Mickey's disassembly carries only the
   underscored label, so the plain spelling is a macro here rather than a
   second symbol. Source bodies keep the SDK's spelling. */
void _bcopy(const void *src, void *dst, int len);
void _bzero(void *dst, int len);
int _bcmp(const void *s1, const void *s2, int len);

#define bcopy _bcopy
#define bzero _bzero
#define bcmp  _bcmp

void osWritebackDCache(void *addr, s32 size);
void osInvalICache(void *addr, s32 size);
s32 __osSiRawReadIo(u32 devAddr, u32 *data);
s32 __osSiRawWriteIo(u32 devAddr, u32 data);
s32 __osSpSetPc(u32 pc);
s32 __osSpRawStartDma(s32 direction, u32 devAddr, void *dramAddr, u32 size);

s32 __osSpDeviceBusy(void);
u32 __osSpGetStatus(void);
void __osSpSetStatus(u32 status);

s32 __osDpDeviceBusy(void);
s32 __osSiDeviceBusy(void);
s32 __osAiDeviceBusy(void);

#define OS_VIM_STACKSIZE 256

#include "PR/os_message.h"

#endif /* _OS_INTERNAL_H_ */
