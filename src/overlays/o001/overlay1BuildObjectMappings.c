#include "PR/ultratypes.h"

typedef struct Overlay1BuildData {
    s8 rank;
    s8 index;
    u8 pad02[0x1A6];
    u16 flags;
} Overlay1BuildData;

typedef struct Overlay1BuildObject {
    u8 pad00[0xC];
    f32 x;
    u8 pad10[4];
    f32 y;
    u8 pad18[0x4C];
    Overlay1BuildData *data;
} Overlay1BuildObject;

typedef struct Overlay1BuildState {
    u8 pad00[0x381];
    s8 byte381;
    s8 byte382;
    s8 byte383;
    s8 byte384;
    u8 pad385[0x1B];
    f32 scale;
    u8 pad3A4[0x5C];
    s32 word400;
    s32 word404;
    s32 word408;
    s32 word40C;
    s32 word410;
} Overlay1BuildState;

extern Overlay1BuildObject **overlay1GetBuildObjectsReloc(s32 *count);
extern void overlay1MarkBuildObjectReloc(Overlay1BuildObject *object);
extern void func_overlay_001_F00004B4_184C894(Overlay1BuildObject *object);
extern void func_overlay_001_F00019B8_184DD98(s32 value);
extern s32 gOverlay1BuildGate;
extern u8 gOverlay1RankBase;
extern u8 gOverlay1RankLimit;
extern u8 D_8[];
extern Overlay1BuildState *D_1DA0;

#ifdef NON_MATCHING
void overlay1BuildObjectMappings(volatile s32 unused) {
    s32 count;
    Overlay1BuildObject **base;
    Overlay1BuildObject **outerCursor;
    Overlay1BuildObject *object;
    Overlay1BuildData *data;
    s32 remaining;
    s32 inner;
    Overlay1BuildObject **innerCursor;
    Overlay1BuildObject *innerObject;
    u8 value;

    base = overlay1GetBuildObjectsReloc(&count);
    if (gOverlay1BuildGate != 0) {
        remaining = count - 1;
        if (count != 0) {
            outerCursor = base + remaining;
            do {
                object = *outerCursor;
                data = object->data;
                if (data->rank >= (gOverlay1RankBase - gOverlay1RankLimit)) {
                    data->flags |= 1;
                    overlay1MarkBuildObjectReloc(object);
                } else {
                    data->flags |= 0x20;
                }
                func_overlay_001_F00004B4_184C894(object);
                D_1DA0->scale = 1.0f;
                D_1DA0->byte381 = 0;
                D_1DA0->byte382 = 0;
                D_1DA0->byte383 = -1;
                D_1DA0->byte384 = 0;
                D_1DA0->word400 = 0;
                *(s16 *)((u8 *)D_1DA0 + 0x3BA) = 0xFF;
                *(f32 *)((u8 *)D_1DA0 + 0x3D0) = object->x;
                *(f32 *)((u8 *)D_1DA0 + 0x3D4) = object->y;
                *(f32 *)((u8 *)D_1DA0 + 0x3D8) = object->x;
                *(f32 *)((u8 *)D_1DA0 + 0x3DC) = object->y;
                if (gOverlay1BuildGate == 1) {
                    func_overlay_001_F00019B8_184DD98(0);
                    D_1DA0->word404 = 0;
                    ((Overlay1BuildState *)((u8 *)D_1DA0 + 4))->word404 = 0;
                    ((Overlay1BuildState *)((u8 *)D_1DA0 + 4))->word408 = 0;
                    ((Overlay1BuildState *)((u8 *)D_1DA0 + 4))->word40C = 0;
                    ((Overlay1BuildState *)((u8 *)D_1DA0 + 4))->word410 = 0;
                }
                inner = count - 1;
                if (count != 0) {
                    innerCursor = base + inner;
                    do {
                        innerObject = *innerCursor;
                        value = D_8[(((((*outerCursor)->data->index << 2) +
                                     (*outerCursor)->data->index)) << 1) +
                                    innerObject->data->index];
                        innerCursor--;
                        *((u8 *)D_1DA0 + 0x3A8 + inner) = value;
                    } while (inner--);
                }
                outerCursor--;
            } while (remaining--);
        }
    }
}

s32 overlay1BuildScheduleCarrier(s32 first, s32 second) {
    first += 1;
    first <<= 2;
    return first + second;
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay1BuildObjectMappings/func_overlay_001_F0001A54_184DE34.s")
#endif
