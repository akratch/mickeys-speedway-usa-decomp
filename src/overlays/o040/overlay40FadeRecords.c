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

/* Bounded reproof: the output-origin spelling below is the best stock object
 * at 98/101 words, first +0xC, frame 0x8. The three differences at
 * +0xC/+0x10/+0x24 are one v0/v1 globalcolor outcome; the temporary-register
 * lane is exact. One allocator trace and the complete 119-flag lattice found
 * no exact object, with canonical -O2 -mips2 tied for best. The object emits
 * all ten runtime HI16/LO16 roles: D_800D6C4C(timer), D_800D6C52(current),
 * D_800D6C50(target), D_800D6C4E(duration), and D_800D6C54(output). Owned
 * Overlay 40 +0x690..+0x824 / ROM 0x1886F40..0x18870D4 excludes separate
 * +0x824..+0x830 padding. Linked equality remains fallback-only. */
#ifdef NON_MATCHING
/* PLATEAU-HANDOFF
 * symbol: overlay40FadeRecords
 * score: 98/101 words
 * frame: 0x8
 * relocations: 10
 * first-mismatch: +0xC
 * summary: declaration-order and lexical-scope probes were flat; one v0/v1 globalcolor web remains
 */
void overlay40FadeRecords(register s32 *enabled, Overlay40FadeContext *context,
                          s32 amount) {
    Overlay40FadeRecord *record;
    Overlay40ColorSource *source;
    Overlay40Color *color;
    Overlay40Vertex *vertex;
    s16 *group;
    s32 current;
    s32 output;
    s32 timer;
    s32 remaining;
    s32 groupRemaining;
    s32 vertexRemaining;

    output = gOverlay40BlendCurrent;
    current = output;
    timer = gOverlay40BlendTimer;
    if (timer != 0) {
        if (amount < timer) {
            gOverlay40BlendTimer = timer - amount;
            current += ((gOverlay40BlendTarget - output) *
                       gOverlay40BlendTimer) / gOverlay40BlendDuration;
        } else {
            gOverlay40BlendTimer = 0;
        }
        gOverlay40BlendOutput = current;
    }

    record = context->records;
    remaining = context->count;
    while (remaining--) {
        if (*enabled != 0) {
            source = record->source;
            if (source != 0) {
                group = source->groups;
                groupRemaining = (record->count + 0xF) >> 4;
                while (groupRemaining--) {
                    *group++ = 0;
                }

                color = source->colors;
                vertexRemaining = record->count;
                vertex = record->vertices;
                while (vertexRemaining--) {
                    vertex->red = (color->red * current) >> 8;
                    vertex->green = (color->green * current) >> 8;
                    vertex->blue = (color->blue * current) >> 8;
                    vertex++;
                    color++;
                }
                record->dirty = 0;
            }
        }
        enabled++;
        record++;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o040/overlay40FadeRecords/func_overlay_040_F0000690_1886F40.s")
#endif
