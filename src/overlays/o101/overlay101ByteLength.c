#include "PR/ultratypes.h"

/* This byte-string loop has no exact counterpart in the pinned DKR objects. */
s32 overlay101ByteLength(u8 *text) {
    s32 length = 0;
    u8 value = *text++;

    if (value != 0) {
        do {
            value = *text;
            length++;
            text++;
        } while (value != 0);
    }
    return length;
}
