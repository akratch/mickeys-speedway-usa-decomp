#include "overlays/overlay_078.h"

/* Overlay 78: two functions, two resident-call relocs, no data or BSS. */

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
