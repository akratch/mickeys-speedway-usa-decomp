#include "PR/ultratypes.h"

/* DKR v77/v80 and JFG have no exact donor for this effect update. */

typedef struct Overlay85Trigger {
    u8 pad00[0x63];
    u8 enabled;
    f32 scale;
} Overlay85Trigger;

typedef struct Overlay85State {
    u8 pad00[4];
    u8 active;
} Overlay85State;

typedef struct Overlay85Timer {
    s16 timer;
    s16 value;
} Overlay85Timer;

typedef struct Overlay85Object {
    u8 pad00[2];
    s16 value2;
    s16 value4;
    u8 pad06[2];
    f32 effectValue;
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x2E];
    s16 type;
    Overlay85Trigger *trigger;
    u8 pad4C[4];
    Overlay85State *state;
    u8 pad54[0x2C];
    s32 mode;
    Overlay85Timer timer;
} Overlay85Object;

extern void overlay85SpawnReloc(s32 kind, f32 x, f32 y, f32 z, s32 flags,
                                 s32 arg5);
extern void overlay85EffectAReloc(f32 value);
extern void overlay85EffectBReloc(f32 value);
extern void overlay85ApplyReloc(Overlay85Object *object, s32 step);

void overlay85Update(Overlay85Object *object, s32 step) {
    volatile s32 reservation;
    Overlay85Trigger *trigger;
    s16 *countdown;
    Overlay85Timer *timer;
    s32 currentTimer;
    s16 type;

    trigger = object->trigger;
    if (trigger != NULL) {
        countdown = &object->timer.timer;
        if (object->timer.timer > 0) {
            *countdown -= step;
        }
        timer = &object->timer;
        currentTimer = *(volatile s16 *)countdown;
        if (currentTimer == 0 && trigger->enabled != 0) {
            if (object->type == 2) {
                overlay85SpawnReloc(0x208, object->x, object->y, object->z, 4,
                                    0);
            } else {
                overlay85SpawnReloc(0x1F, object->x, object->y, object->z, 4,
                                    0);
            }
            timer->value = trigger->scale * 182.0f;
            timer->timer = 10;
            type = object->type;
            if (type == 2) {
                overlay85EffectAReloc(object->effectValue);
                overlay85EffectBReloc(object->effectValue);
                object->mode = 1;
                overlay85ApplyReloc(object, step);
            } else if (type == 0xF0) {
                overlay85EffectAReloc(object->effectValue);
                object->mode = 0x1F;
                overlay85ApplyReloc(object, 2);
            } else if (type == 0x107) {
                object->mode = 1;
                overlay85ApplyReloc(object, step);
            }
        } else if (currentTimer <= 0) {
            timer->timer = 0;
        }
        object->value4 = timer->value;
        object->value2 = timer->value;
        timer->value = (timer->value * -200) >> 8;
    }
    if (object->state != NULL) {
        object->state->active = 1;
    }
}
