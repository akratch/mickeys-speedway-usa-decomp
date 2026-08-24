#include "PR/ultratypes.h"

typedef struct Overlay91TimerState {
    s32 mode;
    s32 timer;
} Overlay91TimerState;

typedef struct Overlay91Object {
    s16 value0;
    u8 pad02[4];
    s16 flags6;
    u8 pad08[4];
    f32 minValue;
    f32 currentValue;
    f32 maxValue;
    u8 pad18[0x4C];
    Overlay91TimerState *state;
    void *graphOwner;
} Overlay91Object;

typedef struct Overlay91GraphHeader {
    u8 pad00[0x2C];
    u8 count;
} Overlay91GraphHeader;

typedef struct Overlay91GraphOwner {
    Overlay91GraphHeader *header;
    u8 pad04[0x48];
    void *records;
} Overlay91GraphOwner;

typedef struct Overlay91GraphRoot {
    Overlay91GraphOwner *owner;
} Overlay91GraphRoot;

typedef struct Overlay91GraphRecord {
    s16 value;
    u8 pad02[2];
    u32 flags;
} Overlay91GraphRecord;

/* One proxy intentionally models the normalized target split object's eleven
 * role calls.  Retail identities are recorded separately in the handoff. */
extern f32 overlay91CallProxy();
extern volatile s32 overlay91GlobalA;
extern s32 overlay91GlobalB;

void overlay91UpdateTimeline(Overlay91Object *object, s32 elapsed) {
    Overlay91TimerState *state;
    s32 animation;
    s32 value;
    s32 fraction;
    u32 count;
    f32 ratio;
    Overlay91GraphOwner *owner;
    Overlay91GraphRecord *record;
    f32 maximum = 144.0f;
    register f32 minimum = -225.0f;

    state = object->state;
    if (elapsed != 0) {
        do {
            switch (state->mode) {
            case 0:
                animation = 0;
                state->timer += elapsed;
                value = state->timer;
                if (value >= 180) {
                    elapsed = value - 180;
                    state->timer = 0;
                    state->mode = 1;
                } else {
                    elapsed = 0;
                }
                break;
            case 1: {
                animation = 0;
                state->timer += elapsed;
                value = state->timer;
                if (value >= 60) {
                    object->minValue = 0.0f;
                    object->currentValue = 0.0f;
                    object->maxValue = maximum;
                    elapsed = value - 60;
                    state->timer = 0;
                    state->mode = 2;
                } else {
                    fraction = (value << 14) / 60;
                    elapsed = 0;
                    ratio = overlay91CallProxy(fraction);
                    object->minValue = ((-(-225.0f)) * ratio) + minimum;
                    object->currentValue =
                        ((0.0f - 0.0f) * ratio) + 0.0f;
                    object->maxValue = object->currentValue + maximum;
                }
                break;
            }
            case 2:
                animation = 0;
                state->timer += elapsed;
                value = state->timer;
                if (value >= 120) {
                    object->value0 = -0x8000;
                    elapsed = value - 120;
                    state->timer = 0;
                    state->mode = 3;
                    overlay91CallProxy(0x12, 0);
                    overlay91CallProxy(0x1D);
                } else {
                    fraction = (value << 14) / 120;
                    elapsed = 0;
                    ratio = overlay91CallProxy(fraction);
                    object->value0 = (s16)(32768.0f * ratio);
                }
                break;
            case 3:
                animation = 1;
                state->timer += elapsed;
                value = state->timer;
                if (value >= 60) {
                    elapsed = value - 60;
                    state->timer = 0;
                    state->mode = 4;
                    overlay91CallProxy(0x13, 0);
                } else {
                    elapsed = 0;
                }
                break;
            case 4:
                animation = 2;
                state->timer += elapsed;
                value = state->timer;
                if (value >= 60) {
                    elapsed = value - 60;
                    state->timer = 0;
                    state->mode = 5;
                    overlay91CallProxy(0x14, 0);
                    overlay91CallProxy(0x1E);
                    overlay91GlobalA = 0;
                } else {
                    elapsed = 0;
                }
                break;
            case 5:
                animation = 3;
                state->timer += elapsed;
                if (state->timer >= 30) {
                    overlay91GlobalA = 0x83;
                } else if (state->timer >= 15) {
                    overlay91GlobalA = 0x84;
                }
                if (state->timer >= 60) {
                    elapsed = state->timer - 60;
                    state->timer = 0;
                    state->mode = 6;
                    overlay91CallProxy(0x15, 0);
                    overlay91CallProxy(0x1F);
                    overlay91GlobalB = 0;
                } else {
                    elapsed = 0;
                }
                break;
            case 6:
                animation = 4;
                state->timer += elapsed;
                value = state->timer;
                if (value >= 60) {
                    elapsed = value - 60;
                    state->timer = 0;
                    state->mode = 7;
                }
                break;
            case 7: {
                animation = 4;
                state->timer += elapsed;
                value = state->timer;
                if (value >= 60) {
                    overlay91CallProxy(object);
                    return;
                } else {
                    fraction = (value << 14) / 60;
                    elapsed = 0;
                    ratio = overlay91CallProxy(fraction);
                    object->minValue = 225.0f + (minimum * ratio);
                    object->currentValue =
                        ((0.0f - 0.0f) * ratio) + 0.0f;
                    object->maxValue = object->currentValue + maximum;
                }
                break;
            }
            }
        } while (elapsed != 0);
    }

    owner = ((Overlay91GraphRoot *)object->graphOwner)->owner;
    if (owner != 0) {
        record = owner->records;
        if (record != 0) {
            count = owner->header->count;
            if (count--) {
                do {
                    if (record->flags & 0x00100000) {
                        record->value = (s16)(animation << 8);
                    }
                    record++;
                } while (count--);
            }
        }
    }
}
