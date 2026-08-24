#include "PR/ultratypes.h"

typedef struct Overlay34Input {
    s16 angleX;
    s16 angleY;
    f32 depth;
    f32 x;
    f32 y;
    f32 z;
    f32 value;
    s16 resourceId;
    u8 mode;
    u8 pad1B;
    s32 word1C;
    s32 word20;
    s32 word24;
    s32 word28;
    s32 word2C;
    s32 word30;
} Overlay34Input;

typedef struct Overlay34Resource {
    u8 pad00[6];
    u16 width;
    u16 height;
} Overlay34Resource;

typedef struct Overlay34Record {
    u8 byte00;
    u8 byte01;
    u8 byte02;
    u8 byte03;
    s16 short04;
    s16 short06;
    s16 short08;
    s16 short0A;
    s16 short0C;
    s16 short0E;
    u8 byte10;
    u8 byte11;
    u8 byte12;
    u8 byte13;
    s16 short14;
    s16 short16;
    s16 short18;
    s16 short1A;
    s16 short1C;
    s16 short1E;
    Overlay34Resource *resource;
    s32 word24;
    s32 word28;
    s32 word2C;
    s32 word30;
    s32 word34;
    s32 word38;
    u8 byte3C;
    u8 byte3D;
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

extern Overlay34Record *gOverlay34Records;
extern Overlay34Record **gOverlay34Pointers;
extern s32 gOverlay34ActiveCount;
extern s32 gOverlay34Count;
extern Overlay34Resource *func_80034448(s16 resourceId);
extern void mathOneFloatPY(Overlay34Input *input, f32 direction[3]);

/* Pinned DKR v77/v80 and JFG searches found no exact donor. */
#ifdef NON_MATCHING
Overlay34Record *overlay34CreateRecord(Overlay34Input *input) {
    Overlay34Record *record;
    Overlay34Record *current;
    Overlay34Record *candidate;
    s32 index;
    s32 width;
    s32 height;

    candidate = NULL;
    if (gOverlay34ActiveCount < gOverlay34Count) {
        if (gOverlay34Count > 0) {
            record = gOverlay34Records;
            index = 0;
            do {
                index++;
                current = record;
                if (record->active == 0) {
                    candidate = current;
                    break;
                }
                record++;
            } while (index < gOverlay34Count);
        }
        if (candidate != NULL) {
            candidate->resource = func_80034448(input->resourceId);
            if (candidate->resource != NULL) {
                width = (candidate->resource->width - 1) << 5;
                height = (candidate->resource->height - 1) << 5;
                candidate->byte00 = 0x40;
                candidate->byte01 = 0;
                candidate->short04 = width;
                candidate->short06 = 0;
                candidate->byte02 = 1;
                candidate->short08 = width;
                candidate->short0A = height;
                candidate->byte03 = 2;
                candidate->short0C = 0;
                candidate->short0E = 0;
                candidate->byte10 = 0x40;
                candidate->byte11 = 1;
                candidate->short14 = width;
                candidate->short16 = height;
                candidate->byte12 = 2;
                candidate->short18 = 0;
                candidate->short1A = 0;
                candidate->byte13 = 3;
                candidate->short1C = 0;
                candidate->short1E = height;
                candidate->word24 = input->word1C;
                candidate->word28 = input->word28;
                candidate->word2C = input->word20;
                candidate->word30 = input->word2C;
                candidate->word34 = input->word24;
                candidate->byte3C = 0;
                candidate->word38 = input->word30;
                candidate->byte3D = input->mode * 6;
                candidate->x1 = input->x;
                candidate->y1 = input->y;
                candidate->z1 = input->z;
                candidate->x2 = input->x;
                candidate->y2 = input->y;
                candidate->z2 = input->z;
                candidate->depth = -input->depth;
                mathOneFloatPY(input, candidate->direction);
                candidate->active = 1;
                candidate->value = input->value;
                gOverlay34Pointers[gOverlay34ActiveCount] = candidate;
                gOverlay34ActiveCount++;
            }
        }
    }
    return candidate;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o034/overlay34CreateRecord/func_overlay_034_F00000D4_188127C.s")
#endif
