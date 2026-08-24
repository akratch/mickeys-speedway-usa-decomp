#include "PR/ultratypes.h"

/* Overlay 78: two functions, two resident-call relocs, no data or BSS. */
typedef struct {
    f32 value;
} Overlay78Record;

typedef struct {
    s16 value;
    u8 pad2[0x62];
    Overlay78Record *record;
} Overlay78Object;

typedef struct {
    u8 pad0[0x0A];
    s16 valueA;
    s16 valueC;
    s16 valueE;
} Overlay78Config;

void overlay78SetTargetReloc(Overlay78Object *object, s32 arg1, s32 arg2, f32 value);
void overlay78UpdateReloc(Overlay78Object *object, f32 current, f32 target);

void overlay78Init(Overlay78Object *object, Overlay78Config *config) {
    Overlay78Record *record;
    f32 target = (f32)config->valueC / 1000.0f;

    record = object->record;
    record->value = (f32)config->valueE / 1000.0f;
    overlay78SetTargetReloc(object, 0, -1, target);
    object->value = config->valueA;
}

void overlay78Update(Overlay78Object *object, s32 updateRate) {
    Overlay78Record *record = object->record;
    overlay78UpdateReloc(object, record->value, (f32)updateRate);
}
