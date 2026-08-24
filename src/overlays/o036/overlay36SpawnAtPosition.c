#include "PR/ultratypes.h"

typedef struct Overlay36PositionObject {
    u8 pad00[0x0C];
    f32 x;
    f32 y;
    f32 z;
} Overlay36PositionObject;

typedef struct Overlay36SpawnRequest {
    s16 kind;
    s16 pad02;
    s16 x;
    s16 y;
    s16 z;
    s16 count;
    Overlay36PositionObject *owner;
} Overlay36SpawnRequest;

typedef struct Overlay36SpawnedObject {
    u8 pad00[0x3C];
    s32 state;
} Overlay36SpawnedObject;

extern u8 gOverlay36AlternateEffects;
extern Overlay36SpawnedObject *overlay36SpawnEffectReloc(
    Overlay36SpawnRequest *request, s32 mode,
    Overlay36PositionObject *owner);

/* Mickey-local reconstruction; the pinned DKR v77/v80 and JFG scans are negative. */
void overlay36SpawnAtPosition(Overlay36PositionObject *object, s32 unused1,
                              s32 unused2, s32 unused3) {
    Overlay36SpawnRequest request;
    Overlay36SpawnedObject *spawned;

    if (gOverlay36AlternateEffects != 0) {
        request.kind = 0xED;
    } else {
        request.kind = 0x90;
    }
    request.x = object->x;
    request.y = object->y;
    request.z = object->z;
    request.count = 1;
    request.owner = object;
    spawned = overlay36SpawnEffectReloc(&request, 1, object);
    if (spawned != 0) {
        spawned->state = 0;
    }
}
