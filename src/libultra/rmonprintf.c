/*
 * PROVENANCE: adapted from Diddy Kong Racing's published libultra
 * src/libc/rmonPrintf.c. Mickey's whole translation-unit text is
 * independently compiler- and link-exact.
 *
 * Flags: DKR's measured -O2 -mips2 -Wab,-r4300_mul -w preset. This TU is
 * compiled directly because asm-processor line metadata changes IDO's
 * schedule even though the file contains no GLOBAL_ASM.
 */

#include "xstdio.h"
#include "PRinternal/macros.h"

#define UNUSED

static char *proutSyncPrintf(UNUSED char *str, UNUSED const char *buf, UNUSED size_t n) {
    return ((char *) 1);
}

void rmonPrintf(const char *format, ...) {
    va_list args;

    va_start(args, format);
    _Printf(proutSyncPrintf, 0, format, args);
    va_end(args);
}
