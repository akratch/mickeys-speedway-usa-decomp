#include "PR/ultratypes.h"

typedef struct Overlay1PoolRecord {
    u8 pad00[0xC0];
    u16 flags;
    u8 padC2[0xA];
} Overlay1PoolRecord;

extern Overlay1PoolRecord *gOverlay1PoolCursor;
extern Overlay1PoolRecord gOverlay1PoolStart[];
extern Overlay1PoolRecord gOverlay1PoolEnd[];
extern s32 gOverlay1PoolGroup;
extern s32 gOverlay1PoolExhausted;

Overlay1PoolRecord *overlay1AllocateRecord(void) {
    Overlay1PoolRecord *cursor;
    Overlay1PoolRecord *result;

    cursor = gOverlay1PoolCursor;
    result = cursor;
    do {
        cursor = (gOverlay1PoolCursor = cursor + 1);
        if (cursor >= gOverlay1PoolEnd) {
            gOverlay1PoolCursor = gOverlay1PoolStart;
            cursor = gOverlay1PoolStart;
        }
        if (result == cursor) {
            gOverlay1PoolExhausted = 1;
            return 0;
        }
    } while ((((u32)*((u8 *)cursor + 0xC1) >> 2) ==
              gOverlay1PoolGroup) && ((cursor->flags & 1) == 0));

    *((u8 *)result + 0xC1) = (*((u8 *)result + 0xC1) & 0xFF03) |
                                (gOverlay1PoolGroup << 2);
    return result;
}
