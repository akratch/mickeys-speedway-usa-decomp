#include "PR/ultratypes.h"

typedef struct Gfx {
    u32 w0;
    u32 w1;
} Gfx;

typedef struct Overlay26Group {
    u8 pad00[0x2A];
    s16 selector;
} Overlay26Group;

typedef struct Overlay26RenderState {
    u8 pad00[0x30];
    s8 enabled;
    u8 pad31[3];
    Overlay26Group group3;
    Overlay26Group group2;
    Overlay26Group group1;
    Overlay26Group group0;
} Overlay26RenderState;

typedef struct Overlay26Node {
    void *resource;
    u32 segment;
    s16 consumed;
} Overlay26Node;

typedef struct Overlay26ResourceChoice {
    u8 pad00[0x68];
    u32 first;
    u32 alternate;
} Overlay26ResourceChoice;

typedef struct Overlay26Context {
    u8 pad00[0x50];
    void *drawData;
    u8 pad54[0x10];
    Overlay26RenderState *render;
    u8 *nodeTable;
} Overlay26Context;

extern void o26PrepareNode(Overlay26Context *, Overlay26Node *, void *, s32);
extern void o26DrawReloc(Gfx **, s32, Overlay26Group *, f32, f32);
extern void o26FlushReloc(Gfx **);
extern void o26FinishReloc(Gfx **);

/* Workbench structure-mismatch: 133/134 instructions, 88 normalized words;
 * 58 aligned residuals (17 structural, 2 schedule, 39 register), first +0x4C.
 * Store order and delayed group initialization improved alignment; constants/negation remain. */
#ifdef NON_MATCHING
void func_overlay_026_F0001158_187B550(Gfx **dl, s32 drawContext,
                                       Overlay26Context *context) {
    Overlay26RenderState *render;
    Overlay26Group *group;
    Overlay26Node *node;
    Overlay26ResourceChoice *choice;
    Gfx *gfx;
    u32 resourceSegment;
    u32 segmentBase;
    u32 triangleCommand;
    u32 fillCommand;
    s32 alphaMask;
    s32 negativeNodeOffset;
    u8 *nodeEntry;
    s32 groupIndex;
    s32 nodeOffset;

    render = context->render;
    alphaMask = -0x100;
    fillCommand = 0xFB000000;
    if (render->enabled != 0) {
        gfx = *dl;
        *dl = gfx + 1;
        gfx->w1 = 0;
        gfx->w0 = 0xE7000000;

        gfx = *dl;
        groupIndex = 3;
        nodeOffset = 0xC;
        *dl = gfx + 1;
        segmentBase = 0x80000000;
        triangleCommand = 0xBF000000;
        gfx->w1 = alphaMask;
        gfx->w0 = fillCommand;

        do {
            if (groupIndex == 3) {
                group = &render->group3;
            } else if (groupIndex == 2) {
                group = &render->group2;
            } else {
                if (groupIndex == 1) {
                    group = &render->group1;
                } else {
                    group = &render->group0;
                }
            }

            negativeNodeOffset = -nodeOffset;
            nodeEntry = context->nodeTable + negativeNodeOffset;
            node = *(Overlay26Node **)(nodeEntry + 0x10);
            choice = (Overlay26ResourceChoice *)node->resource;
            if (group->selector == 0xFF) {
                resourceSegment = choice->first;
            } else {
                resourceSegment = choice->alternate;
            }

            o26PrepareNode(context, node, context->drawData, 0);
            gfx = *dl;
            *dl = gfx + 1;
            gfx->w0 = 0xFA000000;
            gfx->w1 = (group->selector & 0xFF) | alphaMask;
            o26DrawReloc(dl, drawContext, group, 1.0f, 0.0f);

            gfx = *dl;
            *dl = gfx + 1;
            gfx->w0 = triangleCommand;
            gfx->w1 = node->segment + segmentBase;

            gfx = *dl;
            *dl = gfx + 1;
            gfx->w1 = resourceSegment + segmentBase;
            gfx->w0 = 0x06000000;

            gfx = *dl;
            *dl = gfx + 1;
            gfx->w1 = 0;
            gfx->w0 = triangleCommand;
            o26FlushReloc(dl);

            node->consumed = 0;
            nodeOffset -= 4;
        } while (groupIndex--);

        o26FinishReloc(dl);
        gfx = *dl;
        *dl = gfx + 1;
        gfx->w1 = 0xFFFFFFFF;
        gfx->w0 = 0xFA000000;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o026/overlay26DrawGroups/func_overlay_026_F0001158_187B550.s")
#endif
