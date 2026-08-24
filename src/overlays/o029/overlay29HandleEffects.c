#include "PR/ultratypes.h"

typedef struct Vec3f { f32 x, y, z; } Vec3f;

typedef struct Overlay29Record {
    s16 angle0;
    s16 angle1;
    s16 angle2;
    u8 pad06[2];
    f32 scalar;
    Vec3f position;
    Vec3f vector;
    s16 random0;
    s16 random1;
    s16 random2;
    s16 value;
} Overlay29Record;

typedef struct Overlay29State {
    u8 pad00[0xB];
    u8 active;
    u8 pad0C[0x10];
    Vec3f direction;
    Overlay29Record records[4];
} Overlay29State;

typedef struct Overlay29Resource {
    u8 pad00[6];
    u16 flags;
} Overlay29Resource;

typedef struct Overlay29Object {
    s16 angle0;
    s16 angle1;
    s16 angle2;
    s16 flags06;
    f32 scalar;
    Vec3f position;
    u8 pad18[0x30];
    Overlay29Resource *resource;
    u8 pad4C[0x18];
    Overlay29State *state;
    u8 pad68[0x18];
    u32 flags80;
} Overlay29Object;

extern void overlay29ResetReloc(void);
extern s16 overlay29AngleReloc(f32 x, f32 y);
extern f32 overlay29SqrtReloc(f32 value);
extern void overlay29TransformReloc(s16 *angles, Vec3f *vector);
extern s16 overlay29RandomRangeReloc(s32 low, s32 high);
extern void overlay29ActivateReloc(Overlay29Object *object, s32 mode);
extern void overlay29EmitReloc(s32 id, f32 x, f32 y, f32 z, s32 type, s32 arg);

#define INITIALIZE_RECORD(rec_) \
    do { \
        record = (rec_); \
        record->position.x = object->position.x; \
        record->position.y = object->position.y; \
        record->position.z = object->position.z; \
        record->angle0 = object->angle0; \
        record->angle1 = object->angle1; \
        record->angle2 = object->angle2; \
        record->scalar = object->scalar; \
        record->random0 = overlay29RandomRangeReloc(-0x500, 0x500); \
        record->random1 = overlay29RandomRangeReloc(-0x500, 0x500); \
        record->random2 = overlay29RandomRangeReloc(-0x500, 0x500); \
        record->value = 0xFF; \
    } while (0)

#ifdef NON_MATCHING
void func_overlay_029_F00010C4_187E374(Overlay29Object *objectArg, s32 mode) {
    Overlay29Object *object;
    Overlay29Record *record;
    Overlay29State *state;
    s16 angles[2];
    s16 baseAngle;
    s16 verticalAngle;

    object = objectArg;
    state = object->state;
    if ((mode & 1) != 0) {
        overlay29ResetReloc();
    } else if ((mode & 2) != 0) {
        baseAngle = overlay29AngleReloc(state->direction.x, state->direction.z);
        verticalAngle = overlay29AngleReloc(
            overlay29SqrtReloc((state->direction.z * state->direction.z) +
                               (state->direction.x * state->direction.x)),
            state->direction.y);

        angles[1] = verticalAngle + 0x3000;
        angles[0] = baseAngle;
        record = &state->records[0];
        record->vector.x = 0.0f;
        record->vector.y = 0.0f;
        record->vector.z = -10.0f;
        overlay29TransformReloc(angles, &record->vector);
        INITIALIZE_RECORD(record);

        angles[1] = verticalAngle + 0x2000;
        angles[0] = baseAngle;
        record = &state->records[1];
        record->vector.x = 0.0f;
        record->vector.y = 0.0f;
        record->vector.z = -10.0f;
        overlay29TransformReloc(angles, &record->vector);
        INITIALIZE_RECORD(record);

        angles[0] = baseAngle - 0x3000;
        angles[1] = verticalAngle + 0x2000;
        record = &state->records[2];
        record->vector.x = 0.0f;
        record->vector.y = 0.0f;
        record->vector.z = -10.0f;
        overlay29TransformReloc(angles, &record->vector);
        INITIALIZE_RECORD(record);

        angles[0] = baseAngle + 0x3000;
        angles[1] = verticalAngle + 0x2000;
        record = &state->records[3];
        record->vector.x = 0.0f;
        record->vector.y = 0.0f;
        record->vector.z = -10.0f;
        overlay29TransformReloc(angles, &record->vector);
        INITIALIZE_RECORD(record);
        state->active = 1;
    }

    if ((mode & 4) != 0) {
        object->flags80 |= 2;
        overlay29ActivateReloc(object, 1);
    }
    if ((mode & 8) != 0) {
        overlay29EmitReloc(0x27A, object->position.x, object->position.y,
                           object->position.z, 4, 0);
    }
    object->resource->flags &= ~1;
    object->flags06 |= 0x400;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o029/overlay29HandleEffects/func_overlay_029_F00010C4_187E374.s")
#endif
