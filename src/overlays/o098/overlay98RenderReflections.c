#include "PR/ultratypes.h"

typedef struct Gfx { u32 w0, w1; } Gfx;
typedef struct O98Mtx { u8 bytes[0x40]; } O98Mtx;

typedef struct O98ModelData {
    u8 pad00[0x11];
    u8 special;
    u8 pad12[0x3C];
    s8 mode4E;
    u8 pad4F[0x19];
    void *displayListA;
    void *displayListB;
} O98ModelData;

typedef struct O98Node {
    O98ModelData *data;
    void *vertexData;
    s16 useAlternate;
    s16 partIndex;
    u8 pad0C[0x44];
    void *alternateVertexData;
} O98Node;

typedef struct O98Object {
    s16 rotX, rotY, rotZ;
    s16 flags;
    f32 scale;
    f32 x, y, z;
    u8 pad18[0x21];
    u8 alpha;
    s8 nodeIndex;
    u8 pad3B[5];
    s8 *stateTable;
    u8 pad44[0x24];
    O98Node **nodes;
    u8 pad6C[0x27];
    u8 stateIndex;
} O98Object;

typedef struct O98VisibleEntry { O98Object *object; f32 referenceY; } O98VisibleEntry;
typedef struct O98Cursor { O98VisibleEntry *entry; } O98Cursor;
typedef struct O98Context { u8 bytes[0x40]; } O98Context;
typedef struct O98Globals {
    O98Context contexts[2];
    u8 pad80[4];
    s32 visibleCount;
    O98VisibleEntry visibleEntries[0x50];
} O98Globals;
typedef struct O98Transform {
    s16 rotX, rotY, rotZ;
    u16 pad06;
    f32 scale;
    f32 x, y, z;
} O98Transform;

extern void *o98AcquireRenderContextReloc(void);
extern void o98SetupRenderContextReloc(void *, void *);
extern void o98BuildMatrixReloc(O98Transform *, O98Mtx *, s32);
extern void o98CombineMatrixReloc(O98Mtx *, void *, O98Mtx *);
extern void o98LoadMatrixReloc(O98Mtx *, void *);
extern void o98BuildInverseMatrixReloc(O98Transform *, O98Mtx *, s32);
extern void o98EmitObjectReloc(Gfx **, u8 **, s32, O98Object *);
extern void o98RestoreStateReloc(Gfx **);

extern s32 gO98Toggle;
extern O98Globals gO98Globals;
extern u8 gO98SpecialVertices[];

#ifdef NON_MATCHING
void overlay98RenderReflections(Gfx **dl, u8 **matrixHeap, s32 arg2) {
    O98Mtx matrixC;
    O98Mtx matrixB;
    O98Mtx matrixA;
    void *savedDisplayList;
    void * volatile modelDisplayList;
    O98Transform transform;
    O98Transform inverse;
    volatile O98Cursor cursor;
    Gfx *gfx;
    s32 emittedReflection;
    s32 entryIndex;

    gO98Toggle ^= 1;
    savedDisplayList = o98AcquireRenderContextReloc();
    o98SetupRenderContextReloc(savedDisplayList, &gO98Globals.contexts[gO98Toggle]);

    gfx = *dl; *dl = gfx + 1;
    gfx->w0 = 0xB7000000; gfx->w1 = 0x1000;
    emittedReflection = 0;
    entryIndex = 0;
    if (gO98Globals.visibleCount > 0) {
        u32 segmentBase = 0x80000000;
        cursor.entry = gO98Globals.visibleEntries;
        do {
            O98VisibleEntry *entry = cursor.entry;
            O98Object *object = entry->object;
            f32 referenceY = entry->referenceY;
            f32 distance = object->y - referenceY;
            s32 drewObject = 0;
            s32 specialModel = 0;
            O98Node *node;
            O98ModelData *model;
            s8 state;
            volatile s32 savedStateIndex;

            cursor.entry = entry + 1;
            entryIndex++;
            if (distance < 0.0f) distance = -distance;
            if ((object->flags & 0x400) || (object->flags & 0x40)) continue;
            savedStateIndex = object->stateIndex;
            state = object->stateTable[savedStateIndex + 0x1E];
            if (state == 0) {
                node = object->nodes[object->nodeIndex];
                model = node->data;
                if (model->special != 0) specialModel = 1;
                modelDisplayList = (object->alpha == 0xFF) ? model->displayListA : model->displayListB;
                if (model->mode4E == 0) {
                    transform.x = referenceY - distance;
                    transform.y = object->x;
                    transform.z = object->z;
                    transform.scale = -object->scale;
                    transform.rotX = object->rotZ;
                    transform.rotY = object->rotY;
                    transform.rotZ = object->rotX + 0x8000;
                    o98BuildMatrixReloc(&transform, &matrixB, 0);
                    o98CombineMatrixReloc(&matrixB, savedDisplayList, &matrixA);
                    o98LoadMatrixReloc(&matrixA, *matrixHeap);
                    gfx = *dl; *dl = gfx + 1;
                    gfx->w0 = 0x01010040;
                    gfx->w1 = (u32)(*matrixHeap + segmentBase);
                    *matrixHeap += 0x40;
                    drewObject = 1;
                } else if (node->useAlternate == 0) {
                    volatile f32 savedReferenceY;
                    volatile f32 savedDistance;

                    inverse.x = -object->x;
                    inverse.y = -object->y;
                    inverse.z = -object->z;
                    inverse.scale = 1.0f;
                    inverse.rotX = -object->rotZ;
                    inverse.rotY = -object->rotY;
                    inverse.rotZ = -object->rotX;
                    savedReferenceY = referenceY;
                    savedDistance = distance;
                    o98BuildInverseMatrixReloc(&inverse, &matrixC, 0);

                    transform.x = savedReferenceY - savedDistance;
                    transform.y = object->x;
                    transform.z = object->z;
                    transform.scale = -1.0f;
                    transform.rotX = object->rotZ;
                    transform.rotY = object->rotY;
                    transform.rotZ = object->rotX + 0x8000;
                    o98BuildMatrixReloc(&transform, &matrixB, 0);
                    o98CombineMatrixReloc(&matrixC, &matrixB, &matrixA);
                    o98CombineMatrixReloc(&matrixA, savedDisplayList, &matrixA);
                    o98LoadMatrixReloc(&matrixA, *matrixHeap);
                    gfx = *dl; *dl = gfx + 1;
                    gfx->w0 = 0x01000040;
                    gfx->w1 = (u32)(*matrixHeap + segmentBase);
                    *matrixHeap += 0x40;
                    drewObject = 1;
                    emittedReflection = 1;
                }

                if (drewObject) {
                    gfx = *dl; *dl = gfx + 1;
                    gfx->w0 = 0xFA000000; gfx->w1 = object->alpha | ~0xFF;
                    gfx = *dl; *dl = gfx + 1;
                    gfx->w0 = (u32)(((u32)*(void **)((u8 *)node + node->partIndex * 4 + 0xC) + segmentBase) & 0xFFFFFF) | 0xBF000000;
                    gfx->w1 = (u32)node->vertexData + segmentBase;
                    if (specialModel) {
                        if (savedStateIndex) {
                            gfx = *dl; *dl = gfx + 1;
                            gfx->w1 = (u32)gO98SpecialVertices;
                            gfx->w0 = 0x02000050;
                        } else {
                            gfx = *dl; *dl = gfx + 1;
                            gfx->w0 = 0x02000050;
                            gfx->w1 = (u32)*(void **)((u8 *)node + node->partIndex * 4 + 0x50) + segmentBase;
                        }
                    }
                    gfx = *dl; *dl = gfx + 1;
                    gfx->w0 = 0x06000000; gfx->w1 = (u32)modelDisplayList + segmentBase;
                    gfx = *dl; *dl = gfx + 1; gfx->w0 = 0xBF000000; gfx->w1 = 0;
                    gfx = *dl; *dl = gfx + 1; gfx->w0 = 0xBC00000A; gfx->w1 = 0;
                    o98RestoreStateReloc(dl);
                    gfx = *dl; *dl = gfx + 1; gfx->w0 = 0xFA000000; gfx->w1 = 0xFFFFFFFF;
                }
            } else if (state == 1 && !(object->flags & 0x400)) {
                if (emittedReflection) {
                    gfx = *dl; *dl = gfx + 1;
                    gfx->w0 = 0x01000040;
                    gfx->w1 = (u32)&gO98Globals.contexts[gO98Toggle] + segmentBase;
                    emittedReflection = 0;
                }
                {
                    f32 oldY = object->y;
                    object->y = referenceY - distance;
                    object->scale = -object->scale;
                    o98EmitObjectReloc(dl, matrixHeap, arg2, object);
                    object->y = oldY;
                }
            }
        } while (entryIndex < gO98Globals.visibleCount);
    }
    gfx = *dl; *dl = gfx + 1; gfx->w0 = 0xB6000000; gfx->w1 = 0x1000;
    if (emittedReflection) {
        gfx = *dl; *dl = gfx + 1; gfx->w0 = 0x01000040;
        gfx->w1 = (u32)&gO98Globals.contexts[gO98Toggle] + 0x80000000;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o098/overlay98RenderReflections/func_overlay_098_F0000234_18D8BF4.s")
#endif
