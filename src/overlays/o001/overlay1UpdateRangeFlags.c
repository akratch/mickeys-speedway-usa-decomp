#include "PR/ultratypes.h"

typedef struct Overlay1RangeConfig {
    u8 angleHigh;
    u8 horizontalScale;
    u8 verticalScale;
    u8 mode;
    u8 soundId;
} Overlay1RangeConfig;

typedef struct Overlay1RangeState {
    u8 pad000[0x1A8];
    u16 flags;
} Overlay1RangeState;

typedef struct Overlay1HeightData {
    u8 pad000[0x5C];
    f32 height;
} Overlay1HeightData;

typedef struct Overlay1RangeObject {
    u8 pad000[0xC];
    f32 x;
    f32 y;
    f32 z;
    u8 pad018[0x30];
    Overlay1HeightData *heightData;
    u8 pad04C[0x18];
    void *state;
} Overlay1RangeObject;

extern Overlay1RangeObject **overlay1GetObjectListReloc(s32 *count);
extern s32 overlay1GetAngleValueReloc(f32 dz, f32 dx);
extern void overlay1ActivateObjectReloc(Overlay1RangeObject *object);
extern void overlay1PlaySoundReloc(u8 soundId);

void overlay1UpdateRangeFlags(Overlay1RangeObject *object, void *unused) {
    Overlay1RangeConfig *config;
    register s32 clearMask;
    Overlay1RangeObject **objects;
    s32 count;

    config = object->state;
    clearMask = ~8;
    objects = overlay1GetObjectListReloc(&count);
    if (count--) {
        do {
            Overlay1RangeObject *other;
            Overlay1RangeState *otherState;
            f32 dx;
            f32 dz;
            u32 horizontalRange;
            s16 angle;

            other = objects[count];
            otherState = other->state;
            dx = other->x - object->x;
            dz = other->z - object->z;
            horizontalRange = (u32)config->horizontalScale * 10U;
            if ((dx * dx + dz * dz) <
                (f32)(s32)(horizontalRange * horizontalRange)) {
                angle = (s16)(((u32)config->angleHigh << 8) +
                              overlay1GetAngleValueReloc(dz, dx));
                if ((angle < -0x4000) || (angle >= 0x4001)) {
                    f32 otherY;
                    f32 objectY;

                    otherY = other->y;
                    objectY = object->y;
                    if ((objectY <= otherY + other->heightData->height) &&
                        (otherY <= objectY +
                         (f32)(s32)((u32)config->verticalScale * 10U))) {
                        switch (config->mode) {
                            case 0: {
                                u16 flags;
                                flags = otherState->flags;
                                if (!(flags & 8)) {
                                    otherState->flags = flags | 8;
                                }
                                break;
                            }
                            case 1: {
                                u16 flags;
                                flags = otherState->flags;
                                if (flags & 8) {
                                    otherState->flags = flags & clearMask;
                                    overlay1ActivateObjectReloc(other);
                                    overlay1PlaySoundReloc(config->soundId);
                                }
                                break;
                            }
                        }
                    }
                }
            }
        } while (count--);
    }
}
