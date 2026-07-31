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

s32 __osSpDeviceBusy(void);
u32 __osSpGetStatus(void);
void __osSpSetStatus(u32 status);

s32 __osDpDeviceBusy(void);
s32 __osSiDeviceBusy(void);
s32 __osAiDeviceBusy(void);

#endif /* _OS_INTERNAL_H_ */
