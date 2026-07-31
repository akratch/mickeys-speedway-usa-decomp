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

#endif /* _OS_H_ */
