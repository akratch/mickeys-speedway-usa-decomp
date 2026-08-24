#include "PR/ultratypes.h"

typedef struct Overlay40Color {
    u8 red;
    u8 green;
    u8 blue;
} Overlay40Color;

typedef struct Overlay40ColorSource {
    Overlay40Color *colors;
    s16 *groups;
} Overlay40ColorSource;

typedef struct Overlay40Vertex {
    u8 pad00[6];
    s8 red;
    s8 green;
    s8 blue;
    u8 pad09;
} Overlay40Vertex;

typedef struct Overlay40FadeRecord {
    Overlay40Vertex *vertices;
    u8 pad04[0x1C];
    s16 count;
    u8 pad22[0xC];
    u8 dirty;
    u8 pad2F;
    Overlay40ColorSource *source;
    u8 pad34[0xC];
} Overlay40FadeRecord;

typedef struct Overlay40FadeContext {
    u8 pad00[4];
    Overlay40FadeRecord *records;
    u8 pad08[0x12];
    s16 count;
} Overlay40FadeContext;

extern s16 gOverlay40BlendTimer;
extern s16 gOverlay40BlendCurrent;
extern s16 gOverlay40BlendTarget;
extern s16 gOverlay40BlendDuration;
extern s16 gOverlay40BlendOutput;

#ifdef NON_MATCHING
void overlay40FadeRecords(register s32 *enabled, Overlay40FadeContext *context,
                          s32 amount) {
    Overlay40FadeRecord *record;
    Overlay40ColorSource *source;
    Overlay40Color *color;
    Overlay40Vertex *vertex;
    s16 *group;
    s32 timer;
    s32 current;
    s32 output;
    s32 remaining;
    s32 groupRemaining;
    s32 vertexRemaining;

    current = gOverlay40BlendCurrent;
    timer = gOverlay40BlendTimer;
    output = current;
    if (timer != 0) {
        if (amount < timer) {
            gOverlay40BlendTimer = timer - amount;
            output += ((gOverlay40BlendTarget - current) *
                       gOverlay40BlendTimer) / gOverlay40BlendDuration;
        } else {
            gOverlay40BlendTimer = 0;
        }
        gOverlay40BlendOutput = output;
    }

    remaining = context->count - 1;
    record = context->records;
    if (context->count != 0) {
        do {
            if (*enabled != 0) {
                source = record->source;
                if (source != 0) {
                    group = source->groups;
                    groupRemaining = ((record->count + 0xF) >> 4) - 1;
                    if (((record->count + 0xF) >> 4) != 0) {
                        do {
                            *group++ = 0;
                        } while (groupRemaining-- != 0);
                    }

                    color = source->colors;
                    vertex = record->vertices;
                    vertexRemaining = record->count - 1;
                    if (record->count != 0) {
                        do {
                            vertex->red = (color->red * output) >> 8;
                            vertex->green = (color->green * output) >> 8;
                            vertex->blue = (color->blue * output) >> 8;
                            color++;
                            vertex++;
                        } while (vertexRemaining-- != 0);
                    }
                    record->dirty = 0;
                }
            }
            enabled++;
            record++;
        } while (remaining-- != 0);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o040/overlay40FadeRecords/func_overlay_040_F0000690_1886F40.s")
#endif
