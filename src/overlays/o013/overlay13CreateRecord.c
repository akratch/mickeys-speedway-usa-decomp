#include "PR/ultratypes.h"

typedef struct Overlay13Record {
    u8 pad00[4];
    s16 field04;
    u8 active;
    u8 timer;
    f32 value08;
    f32 value0C;
    f32 value10;
    f32 value14;
    f32 value18;
    f32 value1C;
    f32 value20;
    f32 value24;
    f32 value28;
    s32 field2C;
    u8 pad30[0x50];
} Overlay13Record;

extern s32 gOverlay13Active;
extern s32 gOverlay13Enabled;
extern Overlay13Record gOverlay13Records[];
extern void overlay13Initialize(void);

/* Pinned donors contain no hit in this ownership unit. */
Overlay13Record *overlay13CreateRecord(
    f32 value08, f32 value0C, f32 value10, f32 value14,
    f32 value18, f32 value1C, f32 value20, f32 value24) {
    Overlay13Record *record;
    s32 i;

    if (gOverlay13Active == 0) {
        overlay13Initialize();
    }

    record = gOverlay13Records;
    i = 0;
    if (gOverlay13Enabled >= 0x20) {
        return 0;
    }

    for (i = 0; i < 0x20; i++, record++) {
        if (record->active == 0) {
            break;
        }
    }

    if (i < 0x20) {
        record->field04 = 0;
        record->active = 1;
        record->timer = 0x80;
        record->value08 = value08;
        record->value0C = value0C;
        record->value10 = value10;
        record->value14 = value14;
        record->value18 = value18;
        record->value1C = value1C;
        record->value20 = value20;
        record->value24 = value24;
        record->value28 = 0.0f;
        record->field2C = 1;
        gOverlay13Enabled++;
    }

    return record;
}
