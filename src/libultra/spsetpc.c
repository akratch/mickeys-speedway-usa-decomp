/*
 * PROVENANCE: body adapted from Banjo-Kazooie's public libultra
 * lib/ultralib/src/io/spsetpc.c, a permitted retail-derived decompilation
 * under docs/CLEANROOM.md. Mickey's bytes decide the adapted spelling.
 */

#include "PR/rcp.h"

s32 __osSpSetPc(u32 pc) {
    register u32 status = IO_READ(SP_STATUS_REG);

    if (!(status & SP_STATUS_HALT)) {
        return -1;
    }
    IO_WRITE(SP_PC_REG, pc);

    return 0;
}
