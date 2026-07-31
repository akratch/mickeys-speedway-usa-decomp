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

/* Opaque for now: nothing matched so far dereferences a thread, so inventing
   a field layout would be assertion rather than evidence. Give it real members
   when a translation unit needs one. */
typedef struct OSThread_s OSThread;

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

extern u32 __OSGlobalIntMask;

void __osSetGlobalIntMask(OSHWIntr mask);

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

s32 __osSpDeviceBusy(void);
u32 __osSpGetStatus(void);
void __osSpSetStatus(u32 status);

s32 __osDpDeviceBusy(void);
s32 __osSiDeviceBusy(void);
s32 __osAiDeviceBusy(void);

#endif /* _OS_INTERNAL_H_ */
