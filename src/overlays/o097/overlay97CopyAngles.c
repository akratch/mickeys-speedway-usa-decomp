#include "PR/ultratypes.h"

/* DKR source/object scans found no corresponding routine. */
typedef struct Overlay97Angles {
    s16 x, y, z;
} Overlay97Angles;

typedef struct Overlay97AngleInit {
    u8 pad0[0xA];
    s16 z, y, x;
} Overlay97AngleInit;

void overlay97CopyAngles(Overlay97Angles *angles, Overlay97AngleInit *init) {
    angles->x = init->x;
    angles->y = init->y;
    angles->z = init->z;
}
