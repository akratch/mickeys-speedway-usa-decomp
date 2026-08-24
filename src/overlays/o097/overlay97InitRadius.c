#include "PR/ultratypes.h"

/* Radius initializer; exact DKR and JFG scans are negative. */
typedef struct Overlay97Object { u8 pad0[0x84]; f32 radiusSquared; } Overlay97Object;
typedef struct Overlay97InitData { u8 pad0[0xA]; u8 radius; } Overlay97InitData;
void overlay97InitRadius(Overlay97Object *object, Overlay97InitData *init) {
    f32 radius = (f32)init->radius * 8.0f;
    radius *= radius;
    object->radiusSquared = radius;
}
