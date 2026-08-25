#include "PR/ultratypes.h"

typedef struct Overlay43Gfx {
    u32 w0;
    u32 w1;
} Overlay43Gfx;

typedef struct Overlay43ModelNode {
    u8 pad00[0x0A];
    s16 partIndex;
    void *parts[1];
} Overlay43ModelNode;

typedef struct Overlay43VertexData {
    u8 pad00[4];
    void *vertices;
} Overlay43VertexData;

typedef struct Overlay43RenderState {
    u8 pad00[0x20];
    void *framebuffer;
    u8 pad24[4];
    Overlay43Gfx *displayList;
    u8 *matrixHeap;
    Overlay43Gfx *displayListEnd;
    void *modelDisplayList;
    u8 pad38[0x28];
    Overlay43ModelNode *model;
    Overlay43VertexData *vertexData;
    u8 pad68[4];
    u8 transform[0x4D];
    u8 pending;
} Overlay43RenderState;

typedef struct Overlay43RenderLink {
    u8 pad00[0x1C];
    Overlay43RenderState *state;
} Overlay43RenderLink;

typedef struct Overlay43RenderInput {
    u8 pad00[0x4C];
    Overlay43RenderLink *link;
} Overlay43RenderInput;

typedef struct Overlay43RenderEntry {
    u8 pad00[0x44];
    u8 alpha;
} Overlay43RenderEntry;

typedef struct Overlay43RenderLocals {
    u8 pad00[0x60];
    u8 matrixB[0x40];
    u8 matrixA[0x40];
    Overlay43RenderState *state;
    Overlay43VertexData *vertexData;
    u8 padE8[0x10];
    Overlay43Gfx *displayList;
} Overlay43RenderLocals;

#define OVERLAY43_NEXT_GFX(gfx, displayList) \
    do {                                      \
        (gfx) = (displayList);                \
        (displayList) = (gfx) + 1;            \
    } while (0)

extern void func_overlay_043_F0000000_1889FD0();
extern u8 D_0[];
extern u8 D_10[];
extern u8 D_38[];
extern u8 D_78[];
extern s8 D_C8;
extern Overlay43RenderState *D_120[];
extern u8 D_80000000[];
extern u8 D_800000B8[];

/*
 * PLATEAU (2026-08-25): -O2 -mips1 is the best flag group, with an exact
 * 0x158-byte frame but a 32-byte code-size excess, 309 differing positional
 * words, and first mismatch at +0x4. The remaining blocker is the display-list
 * macro expansion/register lifetime around the ninth saved transform value;
 * diagnostic comparison also masks unresolved overlay relocation identities.
 */
#ifdef NON_MATCHING
void func_overlay_043_F0000BE4_188ABB4(
    Overlay43RenderInput *input,
    Overlay43RenderEntry **entries,
    s32 count) {
    Overlay43RenderLocals locals;
    Overlay43RenderState *state;
    Overlay43Gfx *gfx;
    Overlay43ModelNode *model;
    Overlay43RenderEntry **entry;
    u8 *matrixHeap;
    void *segmentAddress;
    s32 offset;

    state = input->link->state;
    model = state->model;
    locals.vertexData = state->vertexData;
    if (D_C8 < 0xF) {
        locals.displayList = state->displayList;
        matrixHeap = state->matrixHeap;
        locals.state = state;
        func_overlay_043_F0000000_1889FD0(
            locals.displayList, D_0, 0x254);
        func_overlay_043_F0000000_1889FD0(&locals.displayList, 0, 0);
        func_overlay_043_F0000000_1889FD0(
            &locals.displayList, 1, locals.state->framebuffer);

        OVERLAY43_NEXT_GFX(gfx, locals.displayList);
        gfx->w1 = 0x300000;
        gfx->w0 = 0xBA001402;
        OVERLAY43_NEXT_GFX(gfx, locals.displayList);
        gfx->w0 = 0xFF88003F;
        gfx->w1 =
            (u32)locals.state->framebuffer + 0x80000000;
        OVERLAY43_NEXT_GFX(gfx, locals.displayList);
        gfx->w1 = 0;
        gfx->w0 = 0xF7000000;
        OVERLAY43_NEXT_GFX(gfx, locals.displayList);
        gfx->w1 = 0x100100;
        gfx->w0 = 0xED000000;
        OVERLAY43_NEXT_GFX(gfx, locals.displayList);
        gfx->w1 = 0;
        gfx->w0 = 0xF60FC0FC;
        func_overlay_043_F0000000_1889FD0(&locals.displayList);

        OVERLAY43_NEXT_GFX(gfx, locals.displayList);
        gfx->w0 = 0xB6000000;
        gfx->w1 = 0x1F3204;
        OVERLAY43_NEXT_GFX(gfx, locals.displayList);
        gfx->w1 = 0;
        gfx->w0 = 0xBB000000;
        OVERLAY43_NEXT_GFX(gfx, locals.displayList);
        gfx->w1 = 0;
        gfx->w0 = 0xBC000002;
        OVERLAY43_NEXT_GFX(gfx, locals.displayList);
        gfx->w0 = 0x03800010;
        gfx->w1 = (u32)D_800000B8;
        OVERLAY43_NEXT_GFX(gfx, locals.displayList);
        gfx->w0 = 0xBC000404;
        gfx->w1 = 2;
        OVERLAY43_NEXT_GFX(gfx, locals.displayList);
        gfx->w1 = 2;
        gfx->w0 = 0xBC000C04;
        OVERLAY43_NEXT_GFX(gfx, locals.displayList);
        gfx->w1 = 0xFFFE;
        gfx->w0 = 0xBC001404;
        OVERLAY43_NEXT_GFX(gfx, locals.displayList);
        gfx->w1 = 0xFFFE;
        gfx->w0 = 0xBC001C04;
        OVERLAY43_NEXT_GFX(gfx, locals.displayList);
        gfx->w0 = 0xED004004;
        gfx->w1 = 0xFC0FC;

        offset = (count - 1) * 4;
        if (count - 1 >= 0) {
            entry = (Overlay43RenderEntry **)((u8 *)entries + offset);
            segmentAddress = D_80000000;
            do {
                u32 color;

                color = (*entry)->alpha;
                color = (color << 24) | (color << 16) | (color << 8);
                OVERLAY43_NEXT_GFX(gfx, locals.displayList);
                gfx->w1 = color | 0xFF;
                gfx->w0 = 0xFA000000;
                OVERLAY43_NEXT_GFX(gfx, locals.displayList);
                gfx->w1 = color | 0xFF;
                gfx->w0 = 0xFB000000;

                func_overlay_043_F0000000_1889FD0(
                    locals.state->transform, *entry, locals.matrixA, color);
                func_overlay_043_F0000000_1889FD0(
                    locals.matrixA, D_78, locals.matrixA);
                func_overlay_043_F0000000_1889FD0(
                    locals.matrixA, D_38, locals.matrixB);
                func_overlay_043_F0000000_1889FD0(
                    locals.matrixB, matrixHeap);

                OVERLAY43_NEXT_GFX(gfx, locals.displayList);
                gfx->w1 = (u32)matrixHeap + 0x80000000;
                gfx->w0 = 0x01000040;
                OVERLAY43_NEXT_GFX(gfx, locals.displayList);
                gfx->w0 =
                    (((u32)model->parts[model->partIndex] + 0x80000000) &
                     0xFFFFFF) |
                    0xBF000000;
                gfx->w1 =
                    (u32)locals.vertexData->vertices + 0x80000000;
                OVERLAY43_NEXT_GFX(gfx, locals.displayList);
                gfx->w0 = 0x02000050;
                gfx->w1 = (u32)segmentAddress;
                OVERLAY43_NEXT_GFX(gfx, locals.displayList);
                gfx->w0 = 0x06000000;
                gfx->w1 =
                    (u32)locals.state->modelDisplayList + 0x80000000;
                OVERLAY43_NEXT_GFX(gfx, locals.displayList);
                gfx->w1 = 0;
                gfx->w0 = 0xBF000000;

                offset -= 4;
                entry--;
                matrixHeap += 0x40;
            } while (offset >= 0);
        }

        OVERLAY43_NEXT_GFX(gfx, locals.displayList);
        gfx->w1 = 0;
        gfx->w0 = 0xBC00000A;
        OVERLAY43_NEXT_GFX(gfx, locals.displayList);
        gfx->w1 = 0;
        gfx->w0 = 0xE9000000;
        OVERLAY43_NEXT_GFX(gfx, locals.displayList);
        gfx->w1 = 0;
        gfx->w0 = 0xB8000000;
        func_overlay_043_F0000000_1889FD0(
            locals.displayList, D_10, 0x2A3);

        locals.state->pending = 1;
        locals.state->displayListEnd = locals.displayList;
        *(s32 *)D_0 = 1;
        D_120[D_C8] = locals.state;
        D_C8++;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o043/func_overlay_043_F0000BE4_188ABB4/func_overlay_043_F0000BE4_188ABB4.s")
#endif
