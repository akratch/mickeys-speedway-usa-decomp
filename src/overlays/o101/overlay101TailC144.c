#include "PR/ultratypes.h"

typedef struct O101TailC144Root {
    u8 pad00[0x1C];
    s32 chainType;
    void *chain;
    u8 kind;
    u8 pad25;
    s16 value26;
    s16 value28;
    s16 value2A;
    s16 value2C;
    s16 width2E;
    s16 height30;
    u8 color32;
    u8 color33;
    void *asset34;
    s32 childType;
    void *child;
    u8 mode40;
    u8 pad41;
    s16 x42;
    s16 width44;
    s16 y46;
    s16 height48;
    s16 value4A;
    s16 value4C;
    u8 color4E;
    u8 color4F;
    void *data50;
} O101TailC144Root;

typedef struct O101TailC144Node32 {
    s32 previousType;
    void *previous;
    s16 x;
    s16 y;
    f32 scale;
    s16 value10;
    u8 color12;
    u8 color13;
    f32 value14;
    s32 value18;
    void *handle;
} O101TailC144Node32;

typedef struct O101TailC144Node24 {
    s32 previousType;
    void *previous;
    s16 x;
    s16 y;
    u8 length;
    s8 opacity;
    u8 mode;
    u8 color0;
    u8 color1;
    u8 color2;
    u8 color3;
    u8 kind;
    u8 *text;
} O101TailC144Node24;

typedef struct O101TailC144Inputs {
    u8 pad000[0x84];
    void *data084;
    u8 pad088[0x128];
    u8 *text1B0;
    u8 *text1B4;
    u8 *text1B8;
    u8 *text1BC;
    u8 *text1C0;
} O101TailC144Inputs;

extern O101TailC144Root gO101TailC144Root;
extern s32 gO101TailC144OrderCount;
extern void *gO101TailC144OrderSlots[];
extern s32 gO101TailC144Node32Count;
extern O101TailC144Node32 gO101TailC144Nodes32[];
extern s32 gO101TailC144Node24Count;
extern O101TailC144Node24 gO101TailC144Nodes24[];
extern O101TailC144Inputs gO101TailC144Inputs;
extern u8 gO101TailC144AssetDCC;
extern u8 gO101TailC144FinalObject4A90;

extern void *func_overlay_101_F0000000_18DB820();
extern s8 func_overlay_101_F000CEA8_18E86C8(void *text);

/* Mickey-local reconstruction; pinned DKR v77/v80 and JFG scans are negative. */
/* Workbench plateau (2026-08-26): structure-mismatch; 361/361 instructions,
 * exact 0x5A4 size/frame, first divergence +0x10, and 246 masked words differ.
 * Levers: two-argument/canonical calls, cast, and declaration/order variants; node-builder drift remains. */
#ifdef NON_MATCHING
void func_overlay_101_F000C144_18E7964(void) {
    O101TailC144Node32 *node32;
    O101TailC144Node24 *node24;
    void *handle;
    s32 *orderCount;
    s32 length;
    s32 nodeIndex;
    s32 orderIndex;

    orderCount = &gO101TailC144OrderCount;
    orderIndex = *orderCount;
    gO101TailC144Root.width2E = 0x140;
    gO101TailC144Root.height30 = 0xF0;
    gO101TailC144Root.asset34 = &gO101TailC144AssetDCC;
    gO101TailC144Root.kind = 4;
    gO101TailC144Root.color32 = 0xFF;
    gO101TailC144Root.color33 = 0xFF;
    gO101TailC144Root.value26 = 0;
    gO101TailC144Root.value28 = 0;
    gO101TailC144Root.value2A = 0;
    gO101TailC144Root.value2C = 0;
    gO101TailC144Root.chainType = 0;
    gO101TailC144Root.chain = NULL;
    gO101TailC144OrderSlots[orderIndex] = &gO101TailC144Root.chainType;
    *orderCount = orderIndex + 1;

    node32 = &gO101TailC144Nodes32[gO101TailC144Node32Count];
    node32->x = 0xF2;
    node32->y = 0x14E;
    node32->value10 = 0;
    node32->color12 = 0xFF;
    node32->color13 = 0;
    node32->value18 = 0;
    node32->scale = 1.0f;
    node32->value14 = 0.0f;
    handle = func_overlay_101_F0000000_18DB820(0x93, NULL);

    nodeIndex = gO101TailC144Node32Count;
    node32 = &gO101TailC144Nodes32[nodeIndex];
    node32->previousType = gO101TailC144Root.chainType;
    node32->previous = gO101TailC144Root.chain;
    gO101TailC144Root.chainType = 2;
    gO101TailC144Root.chain = node32;
    node32->handle = handle;
    gO101TailC144Node32Count = nodeIndex + 1;

    orderCount = &gO101TailC144OrderCount;
    orderIndex = *orderCount;
    gO101TailC144Root.value4A = 0xCC;
    gO101TailC144Root.height48 = 0x51;
    gO101TailC144Root.value4C = 0x4E;
    gO101TailC144Root.width44 = 0x18;
    gO101TailC144Root.x42 = 0x20;
    gO101TailC144Root.y46 = 0x20;
    gO101TailC144Root.color4E = 0xFF;
    gO101TailC144Root.color4F = 0xFF;
    gO101TailC144Root.mode40 = 0;
    gO101TailC144Root.childType = 0;
    gO101TailC144Root.child = NULL;
    gO101TailC144Root.data50 = gO101TailC144Inputs.data084;
    gO101TailC144OrderSlots[orderIndex] = &gO101TailC144Root.childType;
    *orderCount = orderIndex + 1;

    nodeIndex = gO101TailC144Node24Count;
    node24 = &gO101TailC144Nodes24[nodeIndex];
    node24->x = 0x66;
    node24->y = 0x10;
    length = func_overlay_101_F000CEA8_18E86C8(gO101TailC144Inputs.text1B0);
    nodeIndex = gO101TailC144Node24Count;
    node24 = &gO101TailC144Nodes24[nodeIndex];
    node24->length = (u8)length;
    node24->opacity =
        (s8)(s32)((f32)(u32)(length & 0xFF) * (f32)(s32)1);
    node24->mode = 2;
    node24->color0 = 0;
    node24->color1 = 0;
    node24->color2 = 0;
    node24->color3 = 0;
    node24->kind = 4;
    node24->text = gO101TailC144Inputs.text1B0;
    node24->previousType = gO101TailC144Root.childType;
    node24->previous = gO101TailC144Root.child;
    gO101TailC144Root.childType = 3;
    gO101TailC144Root.child = node24;
    gO101TailC144Node24Count = nodeIndex + 1;

    nodeIndex = gO101TailC144Node24Count;
    node24 = &gO101TailC144Nodes24[nodeIndex];
    node24->x = 0x66;
    node24->y = 0x1E;
    length = func_overlay_101_F000CEA8_18E86C8(gO101TailC144Inputs.text1B4);
    nodeIndex = gO101TailC144Node24Count;
    node24 = &gO101TailC144Nodes24[nodeIndex];
    node24->length = (u8)length;
    node24->opacity =
        (s8)(s32)((f32)(u32)(length & 0xFF) * (f32)(s32)1);
    node24->mode = 2;
    node24->color0 = 0;
    node24->color1 = 0;
    node24->color2 = 0;
    node24->color3 = 0;
    node24->kind = 4;
    node24->text = gO101TailC144Inputs.text1B4;
    node24->previousType = gO101TailC144Root.childType;
    node24->previous = gO101TailC144Root.child;
    gO101TailC144Root.childType = 3;
    gO101TailC144Root.child = node24;
    gO101TailC144Node24Count = nodeIndex + 1;

    nodeIndex = gO101TailC144Node24Count;
    node24 = &gO101TailC144Nodes24[nodeIndex];
    node24->x = 0x66;
    node24->y = 0x28;
    length = func_overlay_101_F000CEA8_18E86C8(gO101TailC144Inputs.text1B8);
    nodeIndex = gO101TailC144Node24Count;
    node24 = &gO101TailC144Nodes24[nodeIndex];
    node24->length = (u8)length;
    node24->opacity =
        (s8)(s32)((f32)(u32)(length & 0xFF) * (f32)(s32)1);
    node24->mode = 2;
    node24->color0 = 0;
    node24->color1 = 0;
    node24->color2 = 0;
    node24->color3 = 0;
    node24->kind = 4;
    node24->text = gO101TailC144Inputs.text1B8;
    node24->previousType = gO101TailC144Root.childType;
    node24->previous = gO101TailC144Root.child;
    gO101TailC144Root.childType = 3;
    gO101TailC144Root.child = node24;
    gO101TailC144Node24Count = nodeIndex + 1;

    nodeIndex = gO101TailC144Node24Count;
    node24 = &gO101TailC144Nodes24[nodeIndex];
    node24->x = 0x66;
    node24->y = 0x36;
    length = func_overlay_101_F000CEA8_18E86C8(gO101TailC144Inputs.text1BC);
    nodeIndex = gO101TailC144Node24Count;
    node24 = &gO101TailC144Nodes24[nodeIndex];
    node24->length = (u8)length;
    node24->opacity =
        (s8)(s32)((f32)(u32)(length & 0xFF) * (f32)(s32)1);
    node24->mode = 2;
    node24->color0 = 0;
    node24->color1 = 0;
    node24->color2 = 0;
    node24->color3 = 0;
    node24->kind = 4;
    node24->text = gO101TailC144Inputs.text1BC;
    node24->previousType = gO101TailC144Root.childType;
    node24->previous = gO101TailC144Root.child;
    gO101TailC144Root.childType = 3;
    gO101TailC144Root.child = node24;
    gO101TailC144Node24Count = nodeIndex + 1;

    nodeIndex = gO101TailC144Node24Count;
    node24 = &gO101TailC144Nodes24[nodeIndex];
    node24->x = 0x66;
    node24->y = 0x40;
    length = func_overlay_101_F000CEA8_18E86C8(gO101TailC144Inputs.text1C0);
    nodeIndex = gO101TailC144Node24Count;
    node24 = &gO101TailC144Nodes24[nodeIndex];
    node24->length = (u8)length;
    node24->opacity =
        (s8)(s32)((f32)(u32)(length & 0xFF) * (f32)(s32)1);
    node24->mode = 2;
    node24->color0 = 0;
    node24->color1 = 0;
    node24->color2 = 0;
    node24->color3 = 0;
    node24->kind = 4;
    node24->text = gO101TailC144Inputs.text1C0;
    node24->previousType = gO101TailC144Root.childType;
    node24->previous = gO101TailC144Root.child;
    gO101TailC144Root.childType = 3;
    gO101TailC144Root.child = node24;
    gO101TailC144Node24Count = nodeIndex + 1;

    func_overlay_101_F0000000_18DB820(&gO101TailC144FinalObject4A90);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o101/overlay101TailC144/func_overlay_101_F000C144_18E7964.s")
#endif
