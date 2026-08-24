#include "PR/ultratypes.h"

typedef struct Overlay34Record {
    u8 pad00[0x20];
    void *resource;
    u32 color0A;
    u32 color0B;
    u32 color1A;
    u32 color1B;
    u32 color2A;
    u32 color2B;
    u8 frame;
    u8 frameCount;
    u8 active;
    u8 pad3F;
    f32 x1;
    f32 y1;
    f32 z1;
    f32 x2;
    f32 y2;
    f32 z2;
    f32 direction[2];
    f32 depth;
    f32 value;
} Overlay34Record;

/* Fresh pinned DKR v77/v80 and JFG scans found no Overlay 34 donor. */
extern Overlay34Record **gOverlay34Pointers;
extern s32 gOverlay34ActiveCount;
extern f32 func_80024938(f32 x, f32 y, f32 z);
extern void overlay34InterpolateColor(s32 position, s32 length,
                                      const u8 *start, const u8 *end,
                                      u8 *output);
extern void func_800084C4(s32 arg0, s32 arg1, void *resource,
                          Overlay34Record *record, f32 *position,
                          f32 *secondPosition, f32 value, u32 color1,
                          u32 color2, s32 scale);

#ifdef NON_MATCHING
void overlay34SortAndDraw(s32 arg0, s32 arg1) {
    u32 color1;
    u32 color2;
    f32 distances[64];
    Overlay34Record *record;
    Overlay34Record *swapRecord;
    f32 swapDistance;
    s32 i;
    s32 j;
    s32 offset;
    s32 half;
    s32 position;
    s32 length;

    for (i = 0; i < gOverlay34ActiveCount; i++) {
        record = gOverlay34Pointers[i];
        distances[i] = func_80024938(record->x1, record->y1, record->z1);
    }

    i = gOverlay34ActiveCount;
    while (i--) {
        if (i > 0) {
            for (j = 0; j < i; j++) {
                if (distances[j + 1] < distances[j]) {
                    swapDistance = distances[j];
                    distances[j] = distances[j + 1];
                    distances[j + 1] = swapDistance;
                    swapRecord = gOverlay34Pointers[j];
                    gOverlay34Pointers[j] = gOverlay34Pointers[j + 1];
                    gOverlay34Pointers[j + 1] = swapRecord;
                }
            }
        }
    }

    i = 0;
    offset = 0;
    if (gOverlay34ActiveCount > 0) {
        do {
            i++;
            record = *(Overlay34Record **)((u8 *)gOverlay34Pointers + offset);
            offset += 4;
            if (record != NULL) {
                length = record->frameCount;
                half = length >> 1;
                position = record->frame;
                if (position < half) {
                    overlay34InterpolateColor(position, half,
                                              (u8 *)&record->color0A,
                                              (u8 *)&record->color1A,
                                              (u8 *)&color1);
                    overlay34InterpolateColor(position, half,
                                              (u8 *)&record->color0B,
                                              (u8 *)&record->color1B,
                                              (u8 *)&color2);
                } else {
                    position -= half;
                    length -= half;
                    overlay34InterpolateColor(position, length,
                                              (u8 *)&record->color1A,
                                              (u8 *)&record->color2A,
                                              (u8 *)&color1);
                    overlay34InterpolateColor(position, length,
                                              (u8 *)&record->color1B,
                                              (u8 *)&record->color2B,
                                              (u8 *)&color2);
                }
                func_800084C4(arg0, arg1, record->resource, record,
                              &record->x1, &record->x2, record->value,
                              color1, color2, 0x200);
            }
        } while (i < gOverlay34ActiveCount);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o034/overlay34SortAndDraw/func_overlay_034_F0000608_18817B0.s")
#endif
