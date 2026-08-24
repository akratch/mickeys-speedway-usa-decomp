#include "overlays/overlay019.h"

typedef struct O19Plane {
    f32 x;
    f32 y;
    f32 z;
    f32 d;
} O19Plane;

typedef struct O19ScratchRecord {
    u16 item;
    u16 edgeNeighbor[3];
} O19ScratchRecord;

extern void *o19AllocateReloc(s32 size, s32 tag);
extern void o19FreeReloc(void *value);
extern f32 sqrtf(f32 value);

/* Independent Mickey-only reconstruction, saved before consulting prior work. */
s32 overlay19BuildPlanes(
    O19Context *context, O19Group *group, O19Output *output) {
    register O19Group *g = group;
    register O19Output *o = output;

    O19ScratchRecord *scratch;
    O19ScratchRecord *scratchRecord;
    O19ScratchRecord *record;
    O19Span *span;
    O19Point *point;
    O19Vertex *v0;
    O19Vertex *v1;
    O19Vertex *v2;
    O19Plane *planes;
    O19Plane *plane;
    O19Plane *neighborPlane;
    f32 x0, y0, z0;
    f32 x1, y1, z1;
    f32 x2, y2, z2;
    f32 nx, ny, nz;
    volatile f32 rawNx, rawNy;
    f32 length;
    f32 ex, ey, ez;
    s32 spanIndex;
    s32 spanOffset;
    s32 item;
    s32 edge;
    s32 nextEdge;
    s32 oppositeEdge;
    s32 planeCount;
    s16 itemEnd;
    s16 vertexBase;
    u32 neighbor;

    scratch = o19AllocateReloc(g->itemCount * sizeof(O19ScratchRecord), 0x8A);
    for (item = 0; item < g->itemCount * 2; item++) {
        ((s32 *)scratch)[item] = ((s32 *)o->records)[item];
    }

    planes = (O19Plane *)o->unknown08;
    planeCount = 0;
    spanOffset = 0;
    for (spanIndex = 0; spanIndex < g->spanCount; spanIndex++, spanOffset += sizeof(O19Span)) {
        span = (O19Span *)((u8 *)g->spans + spanOffset);
        item = span->itemStart;
        itemEnd = (span + 1)->itemStart;
        vertexBase = span->vertexBase;
        if (span->flags & 0x1080) {
            item = itemEnd;
        }
        while (item < itemEnd) {
            point = &g->points[item];
            v0 = (O19Vertex *)((u8 *)context->vertices +
                              (point->selectors[0] + vertexBase) * sizeof(O19Vertex));
            x0 = v0->x; y0 = v0->y; z0 = v0->z;
            v0 = (O19Vertex *)((u8 *)context->vertices +
                              (point->selectors[1] + vertexBase) * sizeof(O19Vertex));
            x1 = v0->x; y1 = v0->y; z1 = v0->z;
            v0 = (O19Vertex *)((u8 *)context->vertices +
                              (point->selectors[2] + vertexBase) * sizeof(O19Vertex));
            x2 = v0->x; y2 = v0->y; z2 = v0->z;
            rawNx = ((y1 - y0) * (z2 - z1)) - ((z1 - z0) * (y2 - y1));
            nx = rawNx;
            rawNy = ((z1 - z0) * (x2 - x1)) - ((x1 - x0) * (z2 - z1));
            ny = rawNy;
            nz = ((x1 - x0) * (y2 - y1)) - ((y1 - y0) * (x2 - x1));
            length = sqrtf((rawNx * rawNx) + (rawNy * rawNy) + (nz * nz));
            if (length > 0.0f) {
                nx = rawNx / length;
                ny = rawNy / length;
                nz /= length;
            }
            o->records[item].item = planeCount;
            plane = &planes[planeCount++];
            plane->x = nx;
            plane->y = ny;
            plane->z = nz;
            plane->d = -((x0 * nx) + (y0 * ny) + (z0 * nz));
            item++;
        }
    }

    spanOffset = 0;
    for (spanIndex = 0; spanIndex < g->spanCount; spanIndex++, spanOffset += sizeof(O19Span)) {
        span = (O19Span *)((u8 *)g->spans + spanOffset);
        item = span->itemStart;
        itemEnd = (span + 1)->itemStart;
        vertexBase = span->vertexBase;
        if (span->flags & 0x1080) {
            item = itemEnd;
        }
        while (item < itemEnd) {
            scratchRecord = &scratch[item];
            plane = &planes[o->records[item].item];
            for (edge = 0; edge < 3; edge++) {
                nextEdge = edge + 1;
                if (nextEdge >= 3) nextEdge = 0;
                oppositeEdge = nextEdge + 1;
                if (oppositeEdge >= 3) oppositeEdge = 0;
                neighbor = scratchRecord->edgeNeighbor[edge];
                if (neighbor != 0xFFFF) {
                    if (neighbor == 0xFFFE) {
                        neighborPlane = &planes[o->records[item].item];
                    } else {
                        neighborPlane = &planes[o->records[neighbor].item];
                    }
                    v0 = (O19Vertex *)((u8 *)context->vertices +
                                      (point = &g->points[item],
                                       (point->selectors[edge] + vertexBase) * sizeof(O19Vertex)));
                    v1 = (O19Vertex *)((u8 *)context->vertices +
                                      (point->selectors[nextEdge] + vertexBase) * sizeof(O19Vertex));
                    x0 = v0->x; y0 = v0->y; z0 = v0->z;
                    x1 = v1->x; y1 = v1->y; z1 = v1->z;
                    ex = (((neighborPlane->x + plane->x) * 10.0f) + x0) - x1;
                    ey = (((neighborPlane->y + plane->y) * 10.0f) + y0) - y1;
                    ez = (((neighborPlane->z + plane->z) * 10.0f) + z0) - z1;
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
                        g->points[item].unknown00 |= 1 << edge;
                    } else {
                        s32 halfOffset;
                        record = &scratch[neighbor];
                        halfOffset = 0;
                        while (halfOffset != 6) {
                            if (item == record->edgeNeighbor[halfOffset >> 1]) {
                                o->records[neighbor].edgeNeighbor[halfOffset >> 1] =
                                    planeCount | 0x8000;
                                record->edgeNeighbor[halfOffset >> 1] = 0xFFFF;
                            }
                            halfOffset += 2;
                        }
                        v2 = (O19Vertex *)((u8 *)context->vertices +
                                          (point->selectors[oppositeEdge] + vertexBase) *
                                              sizeof(O19Vertex));
                        if ((neighborPlane->d +
                             ((v2->x * neighborPlane->x) +
                              (v2->y * neighborPlane->y) +
                              (v2->z * neighborPlane->z))) < 0.0f) {
                            g->points[item].unknown00 |= 1 << edge;
                        }
                    }
                    o->records[item].edgeNeighbor[edge] = planeCount;
                    scratchRecord->edgeNeighbor[edge] = 0xFFFF;
                    plane = &planes[planeCount++];
                    plane->x = nx;
                    plane->y = ny;
                    plane->z = nz;
                    plane->d = -((x0 * nx) + (y0 * ny) + (z0 * nz));
                }
            }
            item++;
        }
    }
    o19FreeReloc(scratch);
    return planeCount;
}
