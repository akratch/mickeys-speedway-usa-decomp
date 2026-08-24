#include "ultra64.h"

typedef struct { u8 pad00[4]; s16 x, y, z; u8 valueA, valueB; void *related; } Overlay28Source;
typedef struct Overlay28Work Overlay28Work;
struct Overlay28Work {
    void *related;
    f32 x, y, z, valueA;
    u8 pad14[4];
    f32 valueB, scaleA, scaleB;
    s16 angleA, angleB, stepA, stepB, stepC, stepD;
    u8 object[0xC];
    void (*reset)(Overlay28Work *work);
    s16 field40, bufferIndex;
};
typedef struct {
    u8 pad00[0xC]; f32 x, y, z; u8 pad18[0x16]; s16 angle;
    u8 pad30[0x34]; Overlay28Work *work;
} Overlay28Owner;

extern void ext_o0_36630(void *object);
extern void overlay28ResetBuffer(Overlay28Work *work);

void overlay28InitializeWork(Overlay28Owner *owner, Overlay28Source *source) {
    Overlay28Work *work;

    work = owner->work;
    work->related = source->related;
    work->x = source->x;
    work->y = source->y;
    work->z = source->z;
    work->valueA = source->valueA;
    work->valueB = source->valueB;
    work->angleA = 0;
    work->angleB = 0;
    work->stepA = 0x2000;
    work->stepB = 0x4000;
    work->stepC = 0x1000;
    work->stepD = -0x2000;
    work->reset = overlay28ResetBuffer;
    work->scaleA = 4.0f;
    work->scaleB = 2.0f;
    ext_o0_36630(work->object);
    work->field40 = 0;
    work->bufferIndex = 0;
    overlay28ResetBuffer(work);
    overlay28ResetBuffer(work);
    if (source->related != 0) {
        owner->x = ((Overlay28Owner *) source->related)->x;
        owner->y = ((Overlay28Owner *) source->related)->y;
        owner->z = ((Overlay28Owner *) source->related)->z;
        owner->angle = ((Overlay28Owner *) source->related)->angle;
    }
}
