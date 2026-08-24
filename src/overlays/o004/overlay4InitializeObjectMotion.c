#include "PR/ultratypes.h"

typedef struct Overlay4InitState {
    f32 speed;
    u8 pad04[2];
    s16 phase;
    u8 timer;
    u8 trigger;
} Overlay4InitState;

typedef struct Overlay4InitConfig {
    u8 pad00[0xA];
    s8 speed;
    u8 heading;
    u8 pad0C[2];
    u8 timer;
    u8 pad0F[3];
    u8 outputHeading;
} Overlay4InitConfig;

typedef struct Overlay4InitObject {
    s16 heading;
    s16 outputHeading;
    u8 pad04[0x60];
    Overlay4InitState *state;
} Overlay4InitObject;

extern s32 gOverlay4InitStatus;
extern void func_8005AD64(Overlay4InitObject *object, s32 arg1, s32 arg2,
                          f32 arg3);

void overlay4InitializeObjectMotion(Overlay4InitObject *object,
                                    Overlay4InitConfig *config) {
    Overlay4InitState *state;

    state = object->state;
    object->heading = config->heading << 8;
    object->outputHeading = config->outputHeading << 8;
    state->speed = (f32)(config->speed * 10);
    state->timer = (u8)((f32)config->timer * 6.0f);
    state->trigger = 0;
    state->phase = 0;
    func_8005AD64(object, 0, -1, 0.0f);
    gOverlay4InitStatus = 2;
}
