#include "PR/ultratypes.h"

typedef struct Overlay8ScaleRecord {
    u8 pad00[0x0E];
    u16 scale;
} Overlay8ScaleRecord;

typedef struct Overlay8ScaleSlot {
    Overlay8ScaleRecord *record;
    u32 pad04;
} Overlay8ScaleSlot;

typedef struct Overlay8ScaleContext {
    u8 pad00[0x18];
    Overlay8ScaleSlot *slots;
    u8 pad1C[0x10];
    u8 count;
} Overlay8ScaleContext;

typedef struct Overlay8ScalePair {
    s16 first;
    s16 second;
    u32 selector;
} Overlay8ScalePair;

typedef struct Overlay8ScaleOutput {
    u8 pad00[0x0A];
    s16 outputIndex;
    u8 pad0C[0x40];
    Overlay8ScalePair *pairs;
    s16 *outputs[];
} Overlay8ScaleOutput;

typedef struct Overlay8ScaleState {
    u8 pad000[4];
    f32 position;
    u8 pad008[0x414];
    u32 flags41C;
    u8 pad420[0x0C];
    s32 value42C;
} Overlay8ScaleState;

extern const f32 gOverlay8ScaleLowerReloc;
extern const f32 gOverlay8ScaleUpperReloc;

#ifdef NON_MATCHING
void overlay8ScaleOutputs(void *unused, Overlay8ScaleState *state,
                          Overlay8ScaleContext *context,
                          Overlay8ScaleOutput *output) {
    Overlay8ScalePair *pair;
    Overlay8ScaleRecord *record;
    s16 *cursor;
    s32 i;
    u32 index;
    f32 upperThreshold;
    f32 lowerThreshold;

    pair = output->pairs;
    cursor = output->outputs[output->outputIndex];
    i = 0;

    if (context->count > 0) {
        lowerThreshold = gOverlay8ScaleLowerReloc;
        upperThreshold = gOverlay8ScaleUpperReloc;
        do {
            output = (Overlay8ScaleOutput *)pair->selector;
            index = (u32)output & 0xFF;
            if (!index) {}
            record = context->slots[index].record;

            if (((u32)output & 0x00100000) != 0) {
                if ((state->flags41C & 0x4000) != 0) {
                    if ((state->position >= upperThreshold) &&
                        (state->value42C < -30)) {
                        pair->first = 0x100;
                    } else {
                        pair->first = 0x200;
                    }
                } else if (state->position <= lowerThreshold) {
                    pair->first = 0;
                } else {
                    pair->first = 0x100;
                }
            }

            *cursor++ = (pair->first >> 8) * record->scale;
            if (pair->second >= 0) {
                *cursor++ = (pair->second >> 8) * record->scale;
            }
            i++;
            pair++;
        } while (i < context->count);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o008/overlay8ScaleOutputs/func_overlay_008_F0003368_18610C0.s")
#endif
