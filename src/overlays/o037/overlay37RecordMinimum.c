#include "PR/ultratypes.h"

/* Both local DKR v77/v80 object and semantic-signature scans are negative. */
typedef struct Overlay37Record {
    s32 active;
    f32 minimum;
} Overlay37Record;

extern Overlay37Record gOverlay37Records[];

void overlay37RecordMinimum(s32 index, f32 value) {
    Overlay37Record *record;

    record = &gOverlay37Records[index];
    if (record->active == 0) {
        record->active = 1;
        record->minimum = value;
        return;
    }
    if (value < record->minimum) {
        record->minimum = value;
    }
}
