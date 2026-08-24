#include "overlays/overlay019.h"

#define O19_LOAD_SPAN_FIELDS \
    span = (O19Span *)((u8 *)group->spans + frame.offset.vSpanOffset); \
    firstItem = span->itemStart; \
    itemEnd = (span + 1)->itemStart; \
    vertexBase = span->vertexBase; \
    bit = span->flags
#define O19_ADD_REVERSED(a, b) ((b) + (a))
#define O19_ADVANCE_SPAN \
    frame.index.spanIndex = frame.index.vSpanIndex + 1; \
    frame.offset.spanOffset += sizeof(O19Span)

typedef struct O19SpatialMaskFrame {
    union {
        s32 spanOffset;
        volatile s32 vSpanOffset;
    } offset;
    u32 suppressed;
    u8 unused38[0x30];
    union {
        s32 spanIndex;
        volatile s32 vSpanIndex;
    } index;
    u8 unused6C[0x14];
} O19SpatialMaskFrame;

void overlay19BuildSpatialMasks(O19Context *context, O19Group *group, O19Output *output) {
    O19SpatialMaskFrame frame; s32 item, itemEnd, selector; s16 vertexBase; O19Span *span; O19Point *point; O19Vertex *vertices, *vertex; s16 x, y, z, xMax, xMin, yMax, yMin, zMax, zMin; s16 spanCount, lower, upper, step, binStart, binEnd, firstItem; u32 bit, mask; ;
    frame.index.spanIndex = 0; spanCount = group->spanCount; if ((spanCount > 0)) { ; frame.offset.spanOffset = 0; do { O19_LOAD_SPAN_FIELDS;
    if ((firstItem < itemEnd)) { ; item = firstItem; ; frame.suppressed = bit & 0x1080; do { ; xMax = -32000; yMax = -32000; zMax = -32000; if (frame.suppressed != 0) output->masks[item] = 0; else { ; xMin = 32000; if (xMin); yMin = 32000; zMin = 32000; selector = 0; if (selector); mask = 0; if (mask); point = &group->points[item]; vertices = context->vertices;
    do { vertex = (O19Vertex *)((u8 *)vertices + (u32)(point->selectors[selector] + vertexBase) * 10); x = vertex->x; y = vertex->y; z = vertex->z; if ((xMax < x)) { ; xMax = x; } if ((x < xMin)) { ; xMin = x; } if (yMax < y) yMax = y; if (y < yMin) yMin = y; if (zMax < z) zMax = z; if (z < zMin) zMin = z; ; selector++; } while (selector != 3); ; bit = 1;
    lower = group->xLower;
    upper = group->xUpper;
    step = ((upper - lower) >> 3) + 1;
    binEnd = O19_ADD_REVERSED(lower, step);
    firstItem = 0;
    binStart = lower;
    ;
    do {
        if (!(binEnd < xMin || xMax < binStart)) { ; mask |= bit; }
        binEnd += step; binStart += step; bit <<= 1; firstItem++;
#line 10
        if (firstItem);
    } while (firstItem < 8);
#line 100
    ;
    lower = group->zLower;
    upper = group->zUpper;
    step = ((upper - lower) >> 3) + 1;
    binEnd = O19_ADD_REVERSED(lower, step);
    binStart = lower;
    for (firstItem = 0; firstItem < 8; firstItem++) {
        if (!(binEnd < zMin || zMax < binStart)) { ; mask |= bit; }
        binEnd += step;
        binStart += step;
        bit <<= 1;
        ;
    }
    lower = group->yLower;
    upper = group->yUpper;
    step = ((upper - lower) >> 3) + 1;
    binEnd = O19_ADD_REVERSED(lower, step);
    binStart = lower;
    for (firstItem = 0; firstItem < 8; firstItem++) {
        if (!(binEnd < yMin || yMax < binStart)) { ; mask |= bit; }
        binEnd += step;
        binStart += step;
        bit <<= 1;
        ;
    }
    ; output->masks[item] = mask; ; } item++; ; } while (item < itemEnd);
    spanCount = ((volatile O19Group *)group)->spanCount; }
    O19_ADVANCE_SPAN;
    } while ((frame.index.spanIndex < spanCount)); } }
