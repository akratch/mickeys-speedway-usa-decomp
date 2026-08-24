#include "PR/ultratypes.h"

typedef struct Gfx {
    u32 w0;
    u32 w1;
} Gfx;

typedef struct Overlay29Group {
    u8 pad00[0x2A];
    s16 selector;
} Overlay29Group;

typedef struct Overlay29RenderState {
    u8 pad00[0xB];
    s8 enabled;
    u8 pad0C[0x1C];
    Overlay29Group group3;
    Overlay29Group group2;
    Overlay29Group group1;
    Overlay29Group group0;
} Overlay29RenderState;

typedef struct Overlay29Node {
    void *resource;
    u32 segment;
    s16 consumed;
} Overlay29Node;

typedef struct Overlay29ResourceChoice {
    u8 pad00[0x68];
    u32 first;
    u32 alternate;
} Overlay29ResourceChoice;

typedef struct Overlay29Context {
    u8 pad00[0x64];
    Overlay29RenderState *render;
    u8 *nodeTable;
} Overlay29Context;

extern void func_overlay_029_F0000000_187D2B0(
    Gfx **, s32, Overlay29Group *, f32, f32);
#define overlay29DrawReloc func_overlay_029_F0000000_187D2B0
extern void overlay29FlushReloc(Gfx **);
extern void overlay29FinishReloc(Gfx **);

/* Exact at +0x14C8; DKR v77/v80 and JFG have no exact overlay-29 donor. */
void overlay29DrawGroups(Gfx **dl, s32 drawContext,
                         Overlay29Context *context) {
    Overlay29RenderState *render;
    Overlay29Group *group;
    Overlay29Node *node;
    Overlay29ResourceChoice *choice;
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
        groupIndex = 3;
        gfx = *dl;
        *dl = gfx + 1;
        gfx->w0 = 0xE7000000;
        gfx->w1 = 0;

        gfx = *dl;
        nodeOffset = 0xC;
        *dl = gfx + 1;
        segmentBase = 0x80000000;
        triangleCommand = 0xBF000000;
        gfx->w0 = fillCommand;
        gfx->w1 = alphaMask;

        do {
            if (groupIndex == 3) {
                group = &render->group3;
            } else if (groupIndex == 2) {
                group = &render->group2;
            } else {
                group = &render->group0;
                if (groupIndex == 1) {
                    group = &render->group1;
                }
            }

            negativeNodeOffset = ~nodeOffset;
            nodeEntry = context->nodeTable + negativeNodeOffset + 1;
            node = *(Overlay29Node **)(nodeEntry + 0x10);
            choice = (Overlay29ResourceChoice *)node->resource;
            if (group->selector == 0xFF) {
                resourceSegment = choice->first;
            } else {
                resourceSegment = choice->alternate;
            }

            gfx = *dl;
            *dl = gfx + 1;
            gfx->w0 = 0xFA000000;
            gfx->w1 = (group->selector & 0xFF) | alphaMask;
            overlay29DrawReloc(dl, drawContext, group, 1.0f, 0.0f);

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
            overlay29FlushReloc(dl);

            node->consumed = 0;
            nodeOffset -= 4;
        } while (groupIndex--);

        overlay29FinishReloc(dl);
        gfx = *dl;
        *dl = gfx + 1;
        gfx->w1 = 0xFFFFFFFF;
        gfx->w0 = 0xFA000000;
    }
}
