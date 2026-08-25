#include "PR/ultratypes.h"

typedef struct O57GridCommand {
    u32 w0;
    u32 w1;
} O57GridCommand;

typedef struct O57GridVertex {
    s16 x;
    s16 y;
    s16 z;
    u8 r;
    u8 g;
    u8 b;
    u8 a;
} O57GridVertex;

typedef struct O57GridPointSet {
    f32 point[12];
} O57GridPointSet;

typedef struct O57GridModel {
    u8 pad00[0x40];
    O57GridPointSet *points;
} O57GridModel;

typedef struct O57GridOwner {
    u8 pad00[0x68];
    O57GridModel **model;
} O57GridOwner;

typedef struct O57GridObject {
    u8 pad00[8];
    O57GridOwner *owner;
} O57GridObject;

extern O57GridCommand *gO57GridDisplayListReloc;
extern O57GridVertex *gO57GridVertexCursorReloc;
extern void *gO57GridRenderContextReloc;
extern void *gO57GridResourceF8Reloc;
extern void *gO57GridResourceFCReloc;
extern void *gO57GridResource100Reloc;
extern void *gO57GridResource104Reloc;
extern u8 gO57GridVertexPayloadReloc[];
extern u8 gO57GridTriangles0Reloc[];
extern u8 gO57GridMode4CIndexReloc;
extern s32 gO57GridMode184;
extern s32 gO57GridRecord160;
extern s32 gO57GridAlpha13C;
extern f32 gO57GridBlendA8;
extern f32 gO57GridBlendAC;
extern u8 D_4D4[];
extern f32 D_4E4;

extern void func_800221E8(O57GridCommand **displayList, void *context);
extern void func_800367A4(void *resource, void *arg1, s32 arg2, f32 *arg3, s32 updateRate);
extern O57GridObject *func_800508B4(u8 id);
extern void func_8002109C(O57GridOwner *owner);
extern void func_800349A4(O57GridCommand **displayList, void *resource, s32 mode, s32 flags);

/* Overlay 57 text +0x1020..+0x1978. */
/* Plateau: the best flag-lattice candidate (-O2 -mips3 -32 with
 * -Wo,-loopunroll,2) is size-exact at 0x958 bytes but differs in 574 of 598
 * masked words, first at +0x0. Canonical -O2 -mips2 is 0x20 bytes smaller
 * and differs in 585 words. Typed coordinate-array, scalar, volatile
 * midpoint, record-lifetime, register, scope, and loop-unroll variants show
 * that the remaining blocker is the original 0xF0-frame/s1 allocation and
 * local-coordinate layout, not a missing render batch or vertex operation. */
#ifdef NON_MATCHING
void func_overlay_057_F0001020_18A4C18(s32 updateRate) {
    register O57GridModel *model;
    O57GridObject *object;
    O57GridPointSet *record;
    O57GridCommand *command;
    O57GridVertex *vertex;
    s32 recordOffset;
    s32 renderMode;
    s32 mode;
    s32 i;
    f32 blend;
    f32 x[4];
    f32 y[4];
    f32 z[4];
    f32 x01;
    f32 x02;
    f32 x12;
    f32 x13;
    f32 x23;
    f32 y01;
    f32 y02;
    f32 y12;
    f32 y13;
    f32 y23;
    f32 z01;
    f32 z02;
    f32 z12;
    f32 z13;
    f32 z23;

    model = NULL;
    func_800221E8(&gO57GridDisplayListReloc, gO57GridRenderContextReloc);
    func_800367A4(gO57GridResourceF8Reloc, D_4D4, 2, &D_4E4, updateRate);

    mode = gO57GridMode184;
    if ((mode == 0x2F) || (mode == 0x4B)) {
        object = func_800508B4((u8)mode);
        if ((object != NULL) && (object->owner != NULL)) {
            model = *object->owner->model;
            recordOffset = gO57GridRecord160 * 4;
            renderMode = 4;
            blend = gO57GridBlendA8;
        }
        if (gO57GridMode184 == 0x4B) {
            func_8002109C(object->owner);
            gO57GridAlpha13C = 0xFF;
        }
    } else if (mode == 0x4C) {
        object = func_800508B4(0x4C);
        if ((object != NULL) && (object->owner != NULL)) {
            model = *object->owner->model;
            recordOffset = gO57GridMode4CIndexReloc != 0 ? 4 : 0;
            renderMode = 6;
            blend = gO57GridBlendAC;
        }
        gO57GridAlpha13C = 0xFF;
    }

    if (model != NULL) {
        command = gO57GridDisplayListReloc++;
        command->w0 = 0xFA000000;
        command->w1 = (gO57GridAlpha13C & 0xFF) | ~0xFF;

        command = gO57GridDisplayListReloc++;
        command->w0 =
            ((((((u32)gO57GridVertexCursorReloc + 0x80000000) & 6) | 0x48) & 0xFF) << 16) |
            0x04000062;
        command->w1 = (u32)gO57GridVertexCursorReloc + 0x80000000;

        func_800349A4(&gO57GridDisplayListReloc, gO57GridResourceF8Reloc, renderMode,
                      (s32)(D_4E4 * 65536.0f));
        command = gO57GridDisplayListReloc++;
        command->w0 = 0x05110020;
        command->w1 = (u32)&gO57GridTriangles0Reloc[0];

        func_800349A4(&gO57GridDisplayListReloc, gO57GridResourceFCReloc, renderMode,
                      (s32)(D_4E4 * 65536.0f));
        command = gO57GridDisplayListReloc++;
        command->w0 = 0x05110020;
        command->w1 = (u32)&gO57GridTriangles0Reloc[0x20];

        func_800349A4(&gO57GridDisplayListReloc, gO57GridResource100Reloc, renderMode,
                      (s32)(D_4E4 * 65536.0f));
        command = gO57GridDisplayListReloc++;
        command->w0 = 0x05110020;
        command->w1 = (u32)&gO57GridTriangles0Reloc[0x40];

        func_800349A4(&gO57GridDisplayListReloc, gO57GridResource104Reloc, renderMode,
                      (s32)(D_4E4 * 65536.0f));
        command = gO57GridDisplayListReloc++;
        command->w0 = 0x05110020;
        command->w1 = (u32)&gO57GridTriangles0Reloc[0x60];

        record = (O57GridPointSet *)((u8 *)model->points + recordOffset * 12);
        x[0] = record->point[0] + ((record->point[0] - record->point[9]) * blend);
        y[0] = record->point[1] + ((record->point[1] - record->point[10]) * blend);
        z[0] = record->point[2] + ((record->point[2] - record->point[11]) * blend);
        x[3] = record->point[9] - ((record->point[0] - record->point[9]) * blend);
        y[3] = record->point[10] - ((record->point[1] - record->point[10]) * blend);
        z[3] = record->point[11] - ((record->point[2] - record->point[11]) * blend);

        x[1] = record->point[3] + ((record->point[3] - record->point[6]) * blend);
        y[1] = record->point[4] + ((record->point[4] - record->point[7]) * blend);
        z[1] = record->point[5] + ((record->point[5] - record->point[8]) * blend);
        x[2] = record->point[6] - ((record->point[3] - record->point[6]) * blend);
        y[2] = record->point[7] - ((record->point[4] - record->point[7]) * blend);
        z[2] = record->point[8] - ((record->point[5] - record->point[8]) * blend);

        x01 = x[0] + ((x[1] - x[0]) * 0.5f);
        x02 = x[0] + ((x[2] - x[0]) * 0.5f);
        x12 = x[1] + ((x[2] - x[1]) * 0.5f);
        x13 = x[1] + ((x[3] - x[1]) * 0.5f);
        x23 = x[2] + ((x[3] - x[2]) * 0.5f);
        y01 = y[0] + ((y[1] - y[0]) * 0.5f);
        y02 = y[0] + ((y[2] - y[0]) * 0.5f);
        y12 = y[1] + ((y[2] - y[1]) * 0.5f);
        y13 = y[1] + ((y[3] - y[1]) * 0.5f);
        y23 = y[2] + ((y[3] - y[2]) * 0.5f);
        z01 = z[0] + ((z[1] - z[0]) * 0.5f);
        z02 = z[0] + ((z[2] - z[0]) * 0.5f);
        z12 = z[1] + ((z[2] - z[1]) * 0.5f);
        z13 = z[1] + ((z[3] - z[1]) * 0.5f);
        z23 = z[2] + ((z[3] - z[2]) * 0.5f);

        vertex = gO57GridVertexCursorReloc;
        i = 0;
        do {
            vertex[i].r = 0xFF;
            vertex[i].g = 0xFF;
            vertex[i].b = 0xFF;
            vertex[i].a = 0xFF;
            i++;
        } while (i != 9);

#define O57_GRID_VERTEX(px, py, pz)              \
    do {                                         \
        gO57GridVertexCursorReloc->x = (s16)(px); \
        gO57GridVertexCursorReloc->y = (s16)(py); \
        gO57GridVertexCursorReloc->z = (s16)(pz); \
        gO57GridVertexCursorReloc++;             \
    } while (0)

        O57_GRID_VERTEX(x[0], y[0], z[0]);
        O57_GRID_VERTEX(x01, y01, z01);
        O57_GRID_VERTEX(x[1], y[1], z[1]);
        O57_GRID_VERTEX(x02, y02, z02);
        O57_GRID_VERTEX(x12, y12, z12);
        O57_GRID_VERTEX(x13, y13, z13);
        O57_GRID_VERTEX(x[2], y[2], z[2]);
        O57_GRID_VERTEX(x23, y23, z23);
        O57_GRID_VERTEX(x[3], y[3], z[3]);
#undef O57_GRID_VERTEX
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o057/func_overlay_057_F0001020_18A4C18/func_overlay_057_F0001020_18A4C18.s")
#endif
