#include "PR/ultratypes.h"

typedef struct Overlay8MotionRecord {
    u8 pad000[0xC4];
    void *resource;
    u8 pad0C8[0x38];
    s16 direction;
    s16 activeDirection;
    u8 pad104[4];
    s16 fallbackSign;
    u8 pad10A[0x5E];
    s16 gate168;
    s16 gate16A;
} Overlay8MotionRecord;

extern void func_overlay_008_F0000000_185DD58(void *resource);

/* Mickey-local reconstruction; the donor scans found no exact donor. */
s32 func_overlay_008_F0000E88_185EBE0(void *peer,
                                      Overlay8MotionRecord *record) {
    s16 direction;

    if ((record->gate16A == 0) &&
        (record->activeDirection == 0) &&
        (record->gate168 == 0)) {
        direction = record->direction;
        if (direction != 0) {
            record->activeDirection = -direction;
        } else if (record->fallbackSign < 0) {
            record->activeDirection = -1;
        } else {
            record->activeDirection = 1;
        }

        if (record->resource != NULL) {
            func_overlay_008_F0000000_185DD58(record->resource);
        }
        return 1;
    }
    return 0;
}
