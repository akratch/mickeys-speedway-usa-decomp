#include "PR/ultratypes.h"

typedef struct Overlay1PoolRecord {
    s16 x[32];
    s16 y[32];
    u8 selector[32];
    u8 mode[32];
    union {
        u16 flags;
        struct {
            u8 high;
            u8 type;
        } bytes;
    } header;
    u8 padC2[6];
    u32 value;
} Overlay1PoolRecord;

extern Overlay1PoolRecord *overlay1AllocateRecord(void);
extern f32 sqrtf(f32 value);
extern s32 func_overlay_001_F00078DC_1853CBC(Overlay1PoolRecord *record);
extern Overlay1PoolRecord *overlay1FindBestRecord(void);
extern s16 overlay1AnchorX;
extern s16 overlay1AnchorY;

/* DKR v77/v80 and JFG have no exact donor for this record initializer. */
Overlay1PoolRecord *overlay1CreateRecord(s16 x0, s16 y0, s16 x1, s16 y1) {
    Overlay1PoolRecord *record;
    s32 dx;
    s32 dy;

    record = overlay1AllocateRecord();
    if (record == NULL) {
        return overlay1FindBestRecord();
    }

    record->x[0] = x0;
    record->y[0] = y0;
    record->selector[0] = 0xFF;
    record->mode[0] = 0;
    dx = x1 - x0;
    dy = y1 - y0;
    record->value = (u32)sqrtf((f32)((dx * dx) + (dy * dy)));
    record->header.bytes.type =
        (u8)(((u32)record->header.bytes.type & 0xFF) | 3);
    overlay1AnchorX = x1;
    overlay1AnchorY = y1;

    if (func_overlay_001_F00078DC_1853CBC(record) != 0) {
        do {
            record = overlay1FindBestRecord();
            if (record == NULL) {
                return NULL;
            }
        } while ((record->value != 0) &&
                 (func_overlay_001_F00078DC_1853CBC(record) != 0));
    }
    return record;
}
