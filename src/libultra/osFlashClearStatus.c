/*
 * PROVENANCE: body adapted from Jet Force Gemini's public decompilation,
 * libultra/src/flash/flashreadid.c:osFlashClearStatus. Mickey's symbol,
 * linked bytes, and relocations remain authoritative.
 */

#include "PR/os_pi.h"

#define FLASH_CMD_REG 0x10000
#define FLASH_CMD_STATUS 0xD2000000

extern OSPiHandle D_800D7760;

void osFlashClearStatus(void) {
    osEPiWriteIo(&D_800D7760, D_800D7760.baseAddress | FLASH_CMD_REG,
                 FLASH_CMD_STATUS);
    osEPiWriteIo(&D_800D7760, D_800D7760.baseAddress, 0);
}
