#include "PR/ultratypes.h"

/* Both local DKR v77/v80 object and semantic-signature scans are negative. */
typedef struct Overlay37Record {
    s32 active;
    f32 minimum;
} Overlay37Record;

extern Overlay37Record gOverlay37Records[];

s32 overlay37RecordActive(s32 index) {
    return gOverlay37Records[index].active;
}
