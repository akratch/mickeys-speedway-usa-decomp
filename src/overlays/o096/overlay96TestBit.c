#include "PR/ultratypes.h"

/* DKR source/object scans found no corresponding bit-table query. */
s32 overlay96TestBit(u8 *record, s32 index) {
    u8 *byte;

    if ((index >= 0) && (index < 0x80) && (record != NULL)) {
        byte = record;
        byte += index >> 3;
        if (byte[0xB0] & (1 << (index & 7))) {
            return -1;
        }
    }
    return 0;
}
