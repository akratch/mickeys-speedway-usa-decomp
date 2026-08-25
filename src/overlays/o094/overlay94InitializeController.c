#include "PR/ultratypes.h"

typedef struct Overlay94Record {
    u8 pad00[0x40];
    u8 queryState[1];
} Overlay94Record;

typedef struct Overlay94Entity {
    s32 savedValue;
    u8 pad04[4];
    s16 status;
    s16 recordIndex;
    Overlay94Record *records[1];
} Overlay94Entity;

typedef struct Overlay94State {
    f32 currentValue;
    s8 selector;
    u8 pad05[3];
    s32 angle;
    u8 pad0C[4];
    s16 command;
} Overlay94State;

typedef struct Overlay94Object {
    s16 kind;
    u8 pad02[6];
    f32 scale;
    u8 pad0C[0x44];
    void *renderResource;
    u8 pad54[0x10];
    Overlay94State *state;
    Overlay94Entity **entityRef;
} Overlay94Object;

typedef struct Overlay94Init {
    u8 pad00[0x0A];
    s16 kind;
    s32 selector;
    f32 scale;
} Overlay94Init;

typedef struct Overlay94SavedValue {
    s32 unused[2];
    s32 value;
} Overlay94SavedValue;

extern void overlay94SetupReloc(Overlay94Object *object, s32 arg1, s32 arg2,
                                f32 arg3);
extern void overlay94SetCurrentReloc(Overlay94Object *object, f32 value,
                                     f32 rate);
extern void overlay94InstallReloc(Overlay94Entity *entity, s32 savedValue,
                                  Overlay94Object *object);
extern void overlay94RenderReloc(Overlay94Object *object,
                                 Overlay94Entity *entity,
                                 void *renderResource,
                                 Overlay94Record *record);
extern void overlay94QueryReloc(void *queryState, f32 arg1, f32 arg2, f32 arg3,
                                f32 *out0, f32 *out1, f32 *out2);
extern s32 overlay94AngleReloc(f32 value0, f32 value1);

/* Pinned DKR v77/v80 and JFG object scans found no exact donor. */
void overlay94InitializeController(Overlay94Object *object,
                                   const Overlay94Init *init) {
    Overlay94State *state = object->state;
    Overlay94Entity *entity;
    Overlay94SavedValue saved;
    f32 out0;
    f32 out1;
    f32 out2;

    state->currentValue = 0.0f;
    state->selector = init->selector;
    object->kind = init->kind;
    object->scale = init->scale;

    overlay94SetupReloc(object, 0, -1, 0.0f);
    state->command = 0x2000;

    entity = *object->entityRef;
    saved.value = entity->savedValue;
    overlay94SetCurrentReloc(object, state->currentValue, 0.0f);
    overlay94InstallReloc(entity, saved.value, object);

    overlay94RenderReloc(object, entity, object->renderResource,
                         entity->records[entity->recordIndex]);
    entity->status = 0;

    overlay94QueryReloc(entity->records[entity->recordIndex]->queryState,
                        0.0f, 0.0f, -1.0f, &out0, &out1, &out2);
    state->angle = overlay94AngleReloc(out2, out0);
}
