#include "PR/ultratypes.h"

typedef struct O57DrawCommand {
    u32 w0;
    u32 w1;
} O57DrawCommand;

typedef struct O57DrawVertex {
    s16 x;
    s16 y;
    s16 z;
    u8 r;
    u8 g;
    u8 b;
    u8 a;
} O57DrawVertex;

typedef struct O57Point {
    f32 x;
    f32 y;
    f32 z;
} O57Point;

typedef struct O57PointOwner {
    u8 pad00[0x40];
    O57Point * volatile points;
} O57PointOwner;

typedef struct O57PathObject {
    u8 pad00[0x68];
    O57PointOwner **pointOwner;
} O57PathObject;

typedef struct O57PathEntry {
    u8 pad00[8];
    O57PathObject *object;
} O57PathEntry;

extern O57DrawCommand *gO57DisplayListReloc;
extern void *gO57MatrixReloc;
extern O57DrawVertex *gO57VerticesReloc;
extern u8 gO57TriangleListsReloc[];

extern void *gO57DrawResourceF8Reloc;
extern void *gO57DrawResourceFCReloc;
extern void *gO57DrawResource100Reloc;
extern void *gO57DrawResource104Reloc;
extern u8 gO57DrawAnimation4D4Reloc[];
extern f32 gO57DrawFrame4E4Reloc;
extern s32 gO57DrawPathId184Reloc;
extern s32 gO57DrawGridIndex160Reloc;
extern f32 gO57DrawScaleA8Reloc;
extern f32 gO57DrawScaleACReloc;
extern s32 gO57DrawAlpha13CReloc;
extern u8 gO57DrawAlternateGridReloc;

extern void func_800221E8(O57DrawCommand **displayList, void **matrix);
extern void func_800367A4(void *resource, void *animation, s32 mode,
                          f32 *frame, s32 updateRate);
extern O57PathEntry *func_800508B4(u8 pathId);
extern void func_8002109C(O57PathObject *object);
extern void func_800349A4(O57DrawCommand **displayList, void *resource,
                          s32 mode, s32 parameter);

/* Mickey-local reconstruction. The nearest permitted donor skeleton is only
 * 0.056 similar and supplies no reusable body. */
/*
 * Plateau (2026-08-25, 10 attempts): the complete flag lattice first differs
 * at +0x0.  Its closest size-ranked result, -O2 -mips2 with loopunroll 2, is
 * 8 bytes long and differs in 584 of 598 words; the canonical flags are 44
 * bytes short with the same word count.  Retail has a 0xF0 frame and keeps
 * the long-lived point owner in s1, while this source has a 0xA0 frame and
 * materializes that owner on the stack.  Correcting the point-array stride,
 * path-ID reload, declaration order, volatility, and expression staging did
 * not recover that allocation.  A two-worker, ten-minute -mips2 permuter run
 * improved its numeric score from 53085 to 39130 only through non-credible
 * no-op/cached-expression mutations, so none were adopted.
 */
#ifdef NON_MATCHING
void func_overlay_057_F0001020_18A4C18(s32 updateRate) {
    register void * volatile pointOwner;
    f32 spE0;
    f32 spDC;
    f32 spD8;
    f32 spD4;
    f32 spD0;
    f32 spCC;
    f32 spC8;
    f32 spC4;
    f32 spC0;
    f32 spBC;
    f32 spB8;
    f32 spB4;
    f32 spB0;
    f32 spAC;
    f32 spA8;
    f32 spA4;
    f32 spA0;
    f32 sp9C;
    f32 sp98;
    f32 sp94;
    f32 sp90;
    f32 sp8C;
    f32 sp88;
    f32 sp84;
    f32 sp80;
    f32 differenceZ;
    f32 scale;
    s32 gridIndex;
    s32 renderMode;
    f32 firstX;
    f32 firstY;
    f32 firstZ;
    f32 secondX;
    f32 secondY;
    f32 secondZ;
    O57DrawCommand *command;
    O57DrawVertex *colourVertex;
    O57PathEntry *path;
    s32 pathId;
    register s32 colourGroup;

    pointOwner = 0;
    func_800221E8(&gO57DisplayListReloc, &gO57MatrixReloc);
    func_800367A4(gO57DrawResourceF8Reloc, gO57DrawAnimation4D4Reloc,
                  2, &gO57DrawFrame4E4Reloc, updateRate);

    pathId = gO57DrawPathId184Reloc;
    if ((pathId == 0x2F) || (pathId == 0x4B)) {
        path = func_800508B4(pathId & 0xFF);
        if ((path != 0) && (path->object != 0)) {
            pointOwner = *path->object->pointOwner;
            gridIndex = gO57DrawGridIndex160Reloc * 4;
            renderMode = 4;
            scale = gO57DrawScaleA8Reloc;
        }
        if (gO57DrawPathId184Reloc == 0x4B) {
            func_8002109C(path->object);
            gO57DrawAlpha13CReloc = 0xFF;
        }
    } else if (pathId == 0x4C) {
        path = func_800508B4(0x4C);
        if ((path != 0) && (path->object != 0)) {
            pointOwner = *path->object->pointOwner;
            if (gO57DrawAlternateGridReloc != 0) {
                gridIndex = 4;
            } else {
                gridIndex = 0;
            }
            renderMode = 6;
            scale = gO57DrawScaleACReloc;
        }
        gO57DrawAlpha13CReloc = 0xFF;
    }

    if (pointOwner != 0) {
        command = gO57DisplayListReloc++;
        command->w0 = 0xFA000000;
        command->w1 = (gO57DrawAlpha13CReloc & 0xFF) | ~0xFF;

        command = gO57DisplayListReloc++;
        command->w0 = 0x04000062 |
            ((((((u32)gO57VerticesReloc + 0x80000000) & 6) | 0x48) & 0xFF)
             << 16);
        command->w1 = (u32)gO57VerticesReloc + 0x80000000;

        func_800349A4(&gO57DisplayListReloc, gO57DrawResourceF8Reloc,
                      renderMode, (s32)(gO57DrawFrame4E4Reloc * 65536.0f));
        command = gO57DisplayListReloc++;
        command->w0 = 0x05110020;
        command->w1 = (u32)&gO57TriangleListsReloc[0x00];

        func_800349A4(&gO57DisplayListReloc, gO57DrawResourceFCReloc,
                      renderMode, (s32)(gO57DrawFrame4E4Reloc * 65536.0f));
        command = gO57DisplayListReloc++;
        command->w0 = 0x05110020;
        command->w1 = (u32)&gO57TriangleListsReloc[0x20];

        func_800349A4(&gO57DisplayListReloc, gO57DrawResource100Reloc,
                      renderMode, (s32)(gO57DrawFrame4E4Reloc * 65536.0f));
        command = gO57DisplayListReloc++;
        command->w0 = 0x05110020;
        command->w1 = (u32)&gO57TriangleListsReloc[0x40];

        func_800349A4(&gO57DisplayListReloc, gO57DrawResource104Reloc,
                      renderMode, (s32)(gO57DrawFrame4E4Reloc * 65536.0f));
        command = gO57DisplayListReloc++;
        command->w0 = 0x05110020;
        command->w1 = (u32)&gO57TriangleListsReloc[0x60];

        firstX = (((O57PointOwner *)pointOwner)->points[gridIndex].x -
                  ((O57PointOwner *)pointOwner)->points[gridIndex + 3].x) * scale;
        differenceZ = ((O57PointOwner *)pointOwner)->points[gridIndex].z -
                      ((O57PointOwner *)pointOwner)->points[gridIndex + 3].z;
        firstY = (((O57PointOwner *)pointOwner)->points[gridIndex].y -
                  ((O57PointOwner *)pointOwner)->points[gridIndex + 3].y) * scale;
        spA0 = ((O57PointOwner *)pointOwner)->points[gridIndex].x + firstX;
        firstZ = differenceZ * scale;
        sp90 = ((O57PointOwner *)pointOwner)->points[gridIndex].y + firstY;
        sp80 = ((O57PointOwner *)pointOwner)->points[gridIndex].z + firstZ;
        spAC = ((O57PointOwner *)pointOwner)->points[gridIndex + 3].x - firstX;
        sp9C = ((O57PointOwner *)pointOwner)->points[gridIndex + 3].y - firstY;
        sp8C = ((O57PointOwner *)pointOwner)->points[gridIndex + 3].z - firstZ;

        secondX = (((O57PointOwner *)pointOwner)->points[gridIndex + 1].x -
                   ((O57PointOwner *)pointOwner)->points[gridIndex + 2].x) * scale;
        secondY = (((O57PointOwner *)pointOwner)->points[gridIndex + 1].y -
                   ((O57PointOwner *)pointOwner)->points[gridIndex + 2].y) * scale;
        secondZ = (((O57PointOwner *)pointOwner)->points[gridIndex + 1].z -
                   ((O57PointOwner *)pointOwner)->points[gridIndex + 2].z) * scale;
        spA4 = ((O57PointOwner *)pointOwner)->points[gridIndex + 1].x + secondX;
        sp94 = ((O57PointOwner *)pointOwner)->points[gridIndex + 1].y + secondY;
        sp84 = ((O57PointOwner *)pointOwner)->points[gridIndex + 1].z + secondZ;
        spA8 = ((O57PointOwner *)pointOwner)->points[gridIndex + 2].x - secondX;
        sp98 = ((O57PointOwner *)pointOwner)->points[gridIndex + 2].y - secondY;
        sp88 = ((O57PointOwner *)pointOwner)->points[gridIndex + 2].z - secondZ;

        spE0 = ((spA8 - spA4) * 0.5f) + spA4;
        spDC = ((spAC - spA4) * 0.5f) + spA4;
        spD8 = ((spAC - spA8) * 0.5f) + spA8;
        spD4 = ((sp94 - sp90) * 0.5f) + sp90;
        spD0 = ((sp98 - sp90) * 0.5f) + sp90;
        spCC = ((sp98 - sp94) * 0.5f) + sp94;
        spC8 = ((sp9C - sp94) * 0.5f) + sp94;
        spC4 = ((sp9C - sp98) * 0.5f) + sp98;
        spC0 = ((sp84 - sp80) * 0.5f) + sp80;
        spBC = ((sp88 - sp80) * 0.5f) + sp80;
        spB8 = ((sp88 - sp84) * 0.5f) + sp84;
        spB4 = ((sp8C - sp84) * 0.5f) + sp84;
        spB0 = ((sp8C - sp88) * 0.5f) + sp88;

        colourVertex = gO57VerticesReloc;
        colourGroup = 1;
        colourVertex[0].r = 0xFF;
        colourVertex[0].g = 0xFF;
        colourVertex[0].b = 0xFF;
        colourVertex[0].a = 0xFF;
        do {
            colourGroup += 4;
            colourVertex[2].r = 0xFF;
            colourVertex[2].g = 0xFF;
            colourVertex[2].b = 0xFF;
            colourVertex[2].a = 0xFF;
            colourVertex[3].r = 0xFF;
            colourVertex[3].g = 0xFF;
            colourVertex[3].b = 0xFF;
            colourVertex[3].a = 0xFF;
            colourVertex[4].r = 0xFF;
            colourVertex[4].g = 0xFF;
            colourVertex[4].b = 0xFF;
            colourVertex[4].a = 0xFF;
            colourVertex += 4;
            colourVertex[1].r = 0xFF;
            colourVertex[1].g = 0xFF;
            colourVertex[1].b = 0xFF;
            colourVertex[1].a = 0xFF;
        } while (colourGroup != 9);

#define O57_SET_VERTEX(px, py, pz) \
        do { \
            gO57VerticesReloc->x = (s32)(px); \
            gO57VerticesReloc->y = (s32)(py); \
            gO57VerticesReloc->z = (s32)(pz); \
            gO57VerticesReloc++; \
        } while (0)

        O57_SET_VERTEX(spA0, sp90, sp80);
        O57_SET_VERTEX(((spA4 - spA0) * 0.5f) + spA0, spD4, spC0);
        O57_SET_VERTEX(spA4, sp94, sp84);
        O57_SET_VERTEX(((spA8 - spA0) * 0.5f) + spA0, spD0, spBC);
        O57_SET_VERTEX(spE0, spCC, spB8);
        O57_SET_VERTEX(spDC, spC8, spB4);
        O57_SET_VERTEX(spA8, sp98, sp88);
        O57_SET_VERTEX(spD8, spC4, spB0);
        O57_SET_VERTEX(spAC, sp9C, sp8C);

#undef O57_SET_VERTEX
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o057/func_overlay_057_F0001020_18A4C18/func_overlay_057_F0001020_18A4C18.s")
#endif
