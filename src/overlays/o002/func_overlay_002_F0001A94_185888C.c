#include "PR/ultratypes.h"

typedef struct Overlay2Point {
    s16 x;
    s16 y;
} Overlay2Point;

typedef struct Overlay2Entry {
    Overlay2Point *points;
    s32 pointCount;
    u8 pad8[0x14];
} Overlay2Entry;

typedef struct Overlay2Line {
    f32 x0;
    f32 y0;
    f32 x1;
    f32 y1;
    u16 sourceIndex;
    u16 ownerIndex;
} Overlay2Line;

typedef struct Overlay2Node Overlay2Node;

typedef struct Overlay2Shape {
    Overlay2Point *points;
    s32 pointCount;
    s32 pad8;
    struct Overlay2Shape *next;
    u32 flags;
    Overlay2Node *nodes;
    Overlay2Line *lines;
} Overlay2Shape;

typedef struct Overlay2Hit {
    s16 x;
    s16 y;
    u16 index;
    s16 previous;
    s16 next;
    u16 owner;
    s32 flags;
} Overlay2Hit;

extern Overlay2Entry *overlay1GetEntry(u16 index);
extern void overlay2AdjacentIndices(Overlay2Entry *entry, u16 index,
                                    s16 *previous, s16 *next);
extern s32 overlay2QueryNode(f32 x0, f32 y0, f32 x1, f32 y1,
                             Overlay2Node *node);
extern s32 overlay2ContainsPoint(f32 x, f32 y, Overlay2Shape *shape);

extern Overlay2Node *D_38;
extern Overlay2Line *gOverlay2QueryLinesReloc;
extern f32 gOverlay2QueryX0Reloc;
extern f32 gOverlay2QueryY0Reloc;
extern f32 gOverlay2QueryX1Reloc;
extern f32 gOverlay2QueryY1Reloc;
extern Overlay2Hit *gOverlay2QueryHitReloc;
extern f32 gOverlay2QueryHitXReloc;
extern f32 gOverlay2QueryHitYReloc;
extern f32 gOverlay2QueryBestReloc;
extern u16 gOverlay2QueryIndexReloc;
extern u16 D_72;
extern f32 gOverlay2QueryLimitReloc;
extern f32 gOverlay2QueryResultReloc;

/* Workbench p7 batch 12: allocation-mismatch; exact 217 instructions/-0x40 frame, 27 masked/51 raw words, first stack-home +0x138.
 * Lever: previous/next pointer homes regressed to -0x48; inherited constant, pointY, and bounded-permutation probes remain negative.
 * Remains: integer pool/temp web after relocation-heavy globals and 55 relocation aliases; GLOBAL_ASM stays canonical. */
#ifdef NON_MATCHING
s32 func_overlay_002_F0001A94_185888C(f32 x0, f32 y0, f32 x1, f32 y1,
                                      Overlay2Shape *shape, Overlay2Hit *hit,
                                      s32 previousIndex, u16 shapeIndex) {
    Overlay2Entry *baseEntry;
    Overlay2Entry *hitEntry;
    Overlay2Point *point;
    register s16 pointY;

    baseEntry = overlay1GetEntry(shapeIndex);
    D_38 = shape->nodes;
    gOverlay2QueryLinesReloc = shape->lines;
    gOverlay2QueryX0Reloc = x0;
    gOverlay2QueryY0Reloc = y0;
    gOverlay2QueryX1Reloc = x1;
    gOverlay2QueryY1Reloc = y1;
    gOverlay2QueryBestReloc = gOverlay2QueryLimitReloc;
    gOverlay2QueryHitReloc = hit;

    overlay2QueryNode(x0, y0, x1, y1, D_38);
    if (gOverlay2QueryResultReloc == gOverlay2QueryBestReloc) {
        return 0;
    }

    if (hit != 0) {
        hit->x = (s16)gOverlay2QueryHitXReloc;
        hit->y = (s16)gOverlay2QueryHitYReloc;
        hit->index = gOverlay2QueryIndexReloc;
        hit->owner = D_72;

        hitEntry = overlay1GetEntry(D_72);
        overlay2AdjacentIndices(hitEntry, hit->index,
                                &hit->previous, &hit->next);

        if (previousIndex != -1) {
            point = &baseEntry->points[previousIndex];
            if ((s16)gOverlay2QueryX0Reloc == point->x) {
                pointY = point->y;
                if ((s16)gOverlay2QueryY0Reloc == pointY) {
                    if (overlay2ContainsPoint(
                            ((gOverlay2QueryHitXReloc -
                              gOverlay2QueryX0Reloc) *
                             0.5f) +
                                gOverlay2QueryX0Reloc,
                            ((gOverlay2QueryHitYReloc -
                              gOverlay2QueryY0Reloc) *
                             0.5f) +
                                gOverlay2QueryY0Reloc,
                            shape) == 0) {
                        hit->x = point->x;
                        hit->y = point->y;
                        hit->index = previousIndex;
                        hit->owner = shapeIndex;
                        overlay2AdjacentIndices(baseEntry, previousIndex,
                                                &hit->previous, &hit->next);
                        hit->flags = 0;
                        return 1;
                    }
                }
            }
        }

        point = &hitEntry->points[hit->index];
        if ((point->x == (s16)gOverlay2QueryHitXReloc) &&
            ((s16)gOverlay2QueryHitYReloc == point->y)) {
            hit->flags = 1;
        } else {
            if (overlay2ContainsPoint(
                    ((gOverlay2QueryHitXReloc - gOverlay2QueryX0Reloc) *
                     0.5f) +
                        gOverlay2QueryX0Reloc,
                    ((gOverlay2QueryHitYReloc - gOverlay2QueryY0Reloc) *
                     0.5f) +
                        gOverlay2QueryY0Reloc,
                    shape) == 0) {
                hit->x = (s16)gOverlay2QueryX0Reloc;
                pointY = (s16)gOverlay2QueryY0Reloc;
                hit->y = pointY;
                hit->previous = hit->index;
                hit->index = 0xFF;
                hit->flags = 0;
                return 1;
            } else {
                hit->previous = hit->index;
                hit->index = 0xFF;
                hit->flags = 0;
            }
        }
    }
    return 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o002/func_overlay_002_F0001A94_185888C/func_overlay_002_F0001A94_185888C.s")
#endif
