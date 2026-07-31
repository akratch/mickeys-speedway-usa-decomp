/*
 * libultra: the head of the active-thread queue.
 *
 * ROM 0x72410-0x72420 (VRAM 0x80071810). Byte-identical to DKR's built
 * libultra `os/getactivequeue.c` object -- see the provenance note in
 * symbol_addrs.us.txt, including why this address is the thread-queue
 * accessor and 0x748B0 (identical code) is __osViGetCurrentContext.
 *
 * Carries the project's first %hi/%lo relocation out of *compiled* C: the
 * linker resolves __osActiveQueue against the label splat emits for it in the
 * main segment's data, which is why the symbol is declared in
 * symbol_addrs.us.txt as well as here.
 *
 * Flags: -O1 -mips2 -32 (see the Makefile's per-file block).
 */

#include "PR/os_internal.h"

extern OSThread *__osActiveQueue;

OSThread *__osGetActiveQueue(void) {
    return __osActiveQueue;
}
