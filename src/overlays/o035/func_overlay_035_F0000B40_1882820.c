#include "ultra64.h"

typedef struct O35CollisionVertex {
    s16 x;
    s16 y;
    s16 z;
    u8 pad06[4];
} O35CollisionVertex;

typedef struct O35CollisionTriangle {
    u8 flags;
    u8 selectors[3];
    u8 pad04[0xC];
} O35CollisionTriangle;

typedef struct O35CollisionSpan {
    u8 pad00[6];
    s16 vertexBase;
    s16 triangleStart;
    u8 pad0A[2];
    u32 flags;
} O35CollisionSpan;

typedef struct O35CollisionRecord {
    u16 plane;
    u16 edgeNeighbor[3];
} O35CollisionRecord;

typedef struct O35CollisionPlane {
    f32 x;
    f32 y;
    f32 z;
    f32 d;
} O35CollisionPlane;

typedef struct O35CollisionSegment {
    O35CollisionVertex *vertices;
    O35CollisionTriangle *triangles;
    void *unk08;
    O35CollisionSpan *spans;
    void *unk10;
    void *unk14;
    O35CollisionRecord *records;
    O35CollisionPlane *planes;
    u8 pad20[2];
    s16 triangleCount;
    s16 spanCount;
} O35CollisionSegment;

extern s32 D_o35_skip_collision_edges;
extern void *call_o0_0_2AE30(s32 size, s32 tag);
extern void call_o0_0_2B318(void *value);
extern f32 sqrtf(f32 value);

/*
 * Workbench plateau: structure-mismatch; 516/528 instructions, exact 0x130
 * frame, 486 positional words, first +0x4. Flags, widths, volatility, and
 * expression probes regressed; coordinate spill variants were not productive.
 *
 * PROVENANCE: adapted from Diddy Kong Racing,
 * src/object_models.c (model_init_collision).
 */
#ifdef NON_MATCHING
s32 func_overlay_035_F0000B40_1882820(register O35CollisionSegment *s) {
    O35CollisionRecord *scratch;
    O35CollisionRecord *scratchRecord;
    O35CollisionRecord *record;
    O35CollisionSpan *span;
    O35CollisionTriangle *triangle;
    O35CollisionVertex *v0;
    O35CollisionVertex *v1;
    O35CollisionVertex *v2;
    O35CollisionPlane *plane;
    O35CollisionPlane *neighborPlane;
    f32 x0, y0, z0;
    f32 x1, y1, z1;
    f32 x2, y2, z2;
    f32 nx, ny, nz;
    volatile f32 rawNx, rawNy;
    f32 length;
    f32 ex, ey, ez;
    s32 spanIndex;
    s32 spanOffset;
    s32 copyIndex;
    s32 edge;
    s32 nextEdge;
    s32 oppositeEdge;
    s32 halfOffset;
    s16 triangleIndex;
    s16 triangleEnd;
    s16 vertexBase;
    s16 planeCount;
    u16 neighbor;

    scratch = call_o0_0_2AE30(
        s->triangleCount * (s32)sizeof(O35CollisionRecord), 0x91);
    record = s->records;
    scratchRecord = scratch;
    copyIndex = 0;
    if (s->triangleCount * 4 > 0) {
        do {
            copyIndex++;
            scratchRecord = (O35CollisionRecord *)((u8 *)scratchRecord + 2);
            ((s16 *)scratchRecord)[-1] = *(s16 *)record;
            record = (O35CollisionRecord *)((u8 *)record + 2);
        } while (copyIndex < s->triangleCount * 4);
    }

    planeCount = 0;
    spanOffset = 0;
    for (spanIndex = 0; spanIndex < s->spanCount;
         spanIndex++, spanOffset += sizeof(O35CollisionSpan)) {
        span = (O35CollisionSpan *)((u8 *)s->spans + spanOffset);
        triangleIndex = span->triangleStart;
        triangleEnd = (span + 1)->triangleStart;
        vertexBase = span->vertexBase;
        if (span->flags & 0x1080) {
            triangleIndex = triangleEnd;
        }
        while (triangleIndex < triangleEnd) {
            triangle = &s->triangles[triangleIndex];
            v0 = &s->vertices[triangle->selectors[0] + vertexBase];
            x0 = v0->x;
            y0 = v0->y;
            z0 = v0->z;
            v1 = &s->vertices[triangle->selectors[1] + vertexBase];
            x1 = v1->x;
            y1 = v1->y;
            z1 = v1->z;
            v2 = &s->vertices[triangle->selectors[2] + vertexBase];
            x2 = v2->x;
            y2 = v2->y;
            z2 = v2->z;
            rawNx = ((y1 - y0) * (z2 - z1)) -
                    ((z1 - z0) * (y2 - y1));
            nx = rawNx;
            rawNy = ((z1 - z0) * (x2 - x1)) -
                    ((x1 - x0) * (z2 - z1));
            ny = rawNy;
            nz = ((x1 - x0) * (y2 - y1)) -
                 ((y1 - y0) * (x2 - x1));
            length = sqrtf((rawNx * rawNx) + (rawNy * rawNy) + (nz * nz));
            if (length > 0.0f) {
                nx = rawNx / length;
                ny = rawNy / length;
                nz /= length;
            }
            s->records[triangleIndex].plane = planeCount;
            plane = &s->planes[planeCount++];
            plane->x = nx;
            plane->y = ny;
            plane->z = nz;
            plane->d = -((x0 * nx) + (y0 * ny) + (z0 * nz));
            triangleIndex++;
        }
    }

    if (D_o35_skip_collision_edges != 0) {
        call_o0_0_2B318(scratch);
        return planeCount;
    }

    spanOffset = 0;
    for (spanIndex = 0; spanIndex < s->spanCount;
         spanIndex++, spanOffset += sizeof(O35CollisionSpan)) {
        span = (O35CollisionSpan *)((u8 *)s->spans + spanOffset);
        triangleIndex = span->triangleStart;
        triangleEnd = (span + 1)->triangleStart;
        vertexBase = span->vertexBase;
        if (span->flags & 0x1080) {
            triangleIndex = triangleEnd;
        }
        while (triangleIndex < triangleEnd) {
            triangle = &s->triangles[triangleIndex];
            scratchRecord = &scratch[triangleIndex];
            plane = &s->planes[s->records[triangleIndex].plane];
            for (edge = 0; edge < 3; edge++) {
                nextEdge = edge + 1;
                if (nextEdge >= 3) {
                    nextEdge = 0;
                }
                oppositeEdge = nextEdge + 1;
                if (oppositeEdge >= 3) {
                    oppositeEdge = 0;
                }
                neighbor = scratchRecord->edgeNeighbor[edge];
                if (neighbor != 0xFFFF) {
                    if (neighbor == 0xFFFE) {
                        neighborPlane = &s->planes[s->records[triangleIndex].plane];
                    } else {
                        neighborPlane = &s->planes[s->records[neighbor].plane];
                    }
                    v0 = &s->vertices[triangle->selectors[edge] + vertexBase];
                    v1 = &s->vertices[triangle->selectors[nextEdge] + vertexBase];
                    x0 = v0->x;
                    y0 = v0->y;
                    z0 = v0->z;
                    x1 = v1->x;
                    y1 = v1->y;
                    z1 = v1->z;
                    ex = (((neighborPlane->x + plane->x) * 5.0f) + x0) - x0;
                    ey = (((neighborPlane->y + plane->y) * 5.0f) + y0) - y0;
                    ez = (((neighborPlane->z + plane->z) * 5.0f) + z0) - z0;
                    nx = ((y1 - y0) * ez) - ((z1 - z0) * ey);
                    ny = ((z1 - z0) * ex) - ((x1 - x0) * ez);
                    nz = ((x1 - x0) * ey) - ((y1 - y0) * ex);
                    length = sqrtf((nx * nx) + (ny * ny) + (nz * nz));
                    if (length > 0.0f) {
                        nx /= length;
                        ny /= length;
                        nz /= length;
                    }
                    if (neighbor == 0xFFFE) {
                        triangle->flags |= 1 << edge;
                    } else {
                        record = &scratch[neighbor];
                        halfOffset = 0;
                        while (halfOffset != 6) {
                            if (triangleIndex ==
                                record->edgeNeighbor[halfOffset >> 1]) {
                                s->records[neighbor]
                                    .edgeNeighbor[halfOffset >> 1] =
                                    planeCount | 0x8000;
                                record->edgeNeighbor[halfOffset >> 1] = 0xFFFF;
                            }
                            halfOffset += 2;
                        }
                        v2 = &s->vertices[
                            triangle->selectors[oppositeEdge] + vertexBase];
                        if ((neighborPlane->d +
                             ((v2->x * neighborPlane->x) +
                              (v2->y * neighborPlane->y) +
                              (v2->z * neighborPlane->z))) < 0.0f) {
                            triangle->flags |= 1 << edge;
                        }
                    }
                    s->records[triangleIndex].edgeNeighbor[edge] = planeCount;
                    scratchRecord->edgeNeighbor[edge] = 0xFFFF;
                    plane = &s->planes[planeCount++];
                    plane->x = nx;
                    plane->y = ny;
                    plane->z = nz;
                    plane->d = -((x0 * nx) + (y0 * ny) + (z0 * nz));
                }
            }
            triangleIndex++;
        }
    }
    call_o0_0_2B318(scratch);
    return planeCount;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o035/func_overlay_035_F0000B40_1882820/func_overlay_035_F0000B40_1882820.s")
#endif
