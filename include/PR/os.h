#ifndef _OS_H_
#define _OS_H_

/*
 * Public libultra entry points that matched translation units define.
 * Same policy as os_internal.h: only what is needed, written from scratch,
 * signatures read off the ROM's calling convention.
 */

#include "PR/ultratypes.h"

u32 osAiGetLength(void);
void osDpSetStatus(u32 status);

/*
 * Cache maintenance. Not defined by any matched TU yet -- these are here
 * because main/runlink.c calls them, and calling them without a prototype
 * makes IDO pass the arguments as ints, which is right by accident and wrong
 * the first time a pointer argument matters. Signatures read off the ROM:
 * PatchInstruction (ROM 0x3260C) sets a1 = 4 and leaves a0 pointing at the
 * instruction it just patched for both calls.
 */
void osInvalICache(void *vaddr, s32 nbytes);
void osInvalDCache(void *vaddr, s32 nbytes);
void osWritebackDCache(void *vaddr, s32 nbytes);

#endif /* _OS_H_ */
