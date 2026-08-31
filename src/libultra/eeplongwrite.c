/*
 * Mickey's 0x140-byte EEPROM long-write translation unit has the official SDK
 * call surface and timer delay. The placeholder name is retained so existing
 * callers preserve their authenticated relocation target.
 *
 * PROVENANCE: adapted from official libultra source published in the Banjo-
 * Kazooie decomp at `lib/ultralib/src/io/conteeplongwrite.c`, pinned in
 * `tools/reference-builds.lock`. This is a permitted published decomp source
 * under docs/CLEANROOM.md.
 */

#include "PR/os_internal.h"
#include "PRinternal/controller.h"

#define MICKEY_EEPROM_BLOCK_SIZE 8

extern s32 func_80073490(OSMesgQueue *mq, u8 address, u8 *buffer);

s32 func_8006FEF0(OSMesgQueue *mq, u8 address, u8 *buffer, int length) {
    s32 ret = 0;

    if ((u8)address > 0x40) {
        return -1;
    }

    while (length > 0) {
        ERRCK(func_80073490(mq, address, buffer));
        length -= MICKEY_EEPROM_BLOCK_SIZE;
        address++;
        buffer += MICKEY_EEPROM_BLOCK_SIZE;
        osSetTimer(&__osEepromTimer, 12000 * osClockRate / 1000000, 0,
                   &__osEepromTimerQ, &__osEepromTimerMsg);
        osRecvMesg(&__osEepromTimerQ, NULL, OS_MESG_BLOCK);
    }

    return ret;
}
