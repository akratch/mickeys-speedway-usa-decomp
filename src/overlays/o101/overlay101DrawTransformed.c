#include "PR/ultratypes.h"

typedef struct Overlay101Gfx {
    u32 w0;
    u32 w1;
} Overlay101Gfx;

typedef struct Overlay101DrawNode {
    u8 pad00[8];
    u8 type;
    u8 pad09[5];
    s16 x;
    s16 y;
} Overlay101DrawNode;

typedef struct Overlay101TransformElement {
    u8 pad00[8];
    s16 x;
    s16 y;
    f32 scale;
    s16 rotation;
    u8 color;
    u8 pad13;
    f32 depth;
    u8 pad18[4];
    void *object;
} Overlay101TransformElement;

typedef struct Overlay101Transform {
    s16 rotateZ;
    s16 rotateY;
    s16 rotateX;
    s16 pad06;
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x10];
    f32 depth;
} Overlay101Transform;

void overlay101GetDimensions2Reloc(s32 *width, s32 *height);
void overlay101GetBounds2Reloc(Overlay101DrawNode *node, s32 *left, s32 *top,
                               s32 *right, s32 *bottom);
void overlay101SetScissor2Reloc(Overlay101Gfx **displayList, s32 left, s32 top,
                                s32 right, s32 bottom);
void overlay101SetTransformModeReloc(s32 mode);
void overlay101SubmitTransformReloc(Overlay101Gfx **displayList, void *matrix,
                                    void *vertices,
                                    Overlay101Transform *transform,
                                    void *object, s32 rotated, s32 color);

/*
 * Overlay 101 text +0x29A4..+0x2C3C. Natural source supplies the exact size,
 * 0x90 frame, ABI, CFG, FP topology, stack homes, and seven call sites. A
 * scoped decoded ledger selects two retail command schedules and complete
 * equivalent private temporary-register webs. Plateau after the full flag
 * lattice and a command-expression audit: the best 166-word candidate has 62
 * positional differences from first mismatch +0x7C. Removing the otherwise
 * unused nested-assignment temporary shrinks the frame to 0x88 and regresses
 * to 129 words; splitting its assignment preserves the same 62-word basin.
 * The blocker is the two command schedules and the private temporary FIFO.
 */
#ifdef NON_MATCHING
void overlay101DrawTransformed(Overlay101Gfx **displayList, void *matrix,
                               void *vertices, Overlay101DrawNode *node,
                               Overlay101TransformElement *element) {
    s32 bounds0;
    s32 bounds1;
    s32 bounds2;
    s32 bounds3;
    s32 screenWidth;
    s32 screenHeight;
    s32 rotated;
    Overlay101Transform transform;
    Overlay101Gfx *new_var;
    s32 right;
    s32 bottom;
    Overlay101Gfx *command;

    if (((element->color != 0) && (!(element->scale <= 0.0f))) &&
        ((node->type == 2) || (node->type == 4))) {
        overlay101GetDimensions2Reloc(&screenWidth, &screenHeight);
        transform.rotateZ = 0;
        transform.rotateY = 0;
        transform.rotateX = element->rotation;
        transform.scale = element->scale;
        transform.x = ((f32)(node->x + element->x)) -
                      ((f32)(((u32)screenWidth) >> 1));
        transform.y = ((f32)(((u32)screenHeight) >> 1)) -
                      ((f32)(node->y + element->y));
        transform.z = 0.0f;
        transform.depth = element->depth;
        command = (*displayList)++;
        command->w1 = 0;
        command->w0 = 0xE7000000;
        command = (new_var = (*displayList)++);
        command->w0 = 0xFA000000;
        command->w1 = element->color | 0xFFFFFF00;
        command = (*displayList)++;
        command->w1 = 0xFFFFFF00;
        command->w0 = 0xFB000000;
        if (element->rotation != 0)
            rotated = 1;
        else
            rotated = 0;
        overlay101GetBounds2Reloc(node, &bounds0, &bounds1, &bounds2, &bounds3);
        overlay101SetScissor2Reloc(displayList, bounds0, bounds1, bounds2,
                                   bounds3);
        if (element->scale == 1.0f)
            overlay101SetTransformModeReloc(0);
        overlay101SubmitTransformReloc(displayList, matrix, vertices,
                                       &transform, element->object, rotated,
                                       element->color);
        if (element->scale == 1.0f)
            overlay101SetTransformModeReloc(1);
        command = (*displayList)++;
        command->w1 = 0;
        command->w0 = 0xE7000000;
        command = (*displayList)++;
        command->w1 = 0xFFFFFFFF;
        command->w0 = 0xFA000000;
        overlay101SetScissor2Reloc(displayList, 0, 0, 1000, 1000);
        (void)right;
        (void)bottom;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o101/overlay101DrawTransformed/func_overlay_101_F00029A4_18DE1C4.s")
#endif
