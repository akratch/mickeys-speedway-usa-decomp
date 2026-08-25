#include "PR/ultratypes.h"

typedef struct O101TailBA34Root {
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
    s32 tertiaryType;
    void *tertiary;
    u8 mode5C;
    u8 pad5D;
    s16 x5E;
    s16 width60;
    s16 y62;
    s16 height64;
    s16 value66;
    s16 value68;
    u8 color6A;
    u8 color6B;
    void *data6C;
} O101TailBA34Root;

typedef struct O101TailBA34Node32 {
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
} O101TailBA34Node32;

typedef struct O101TailBA34Node24 {
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
} O101TailBA34Node24;

typedef struct O101TailBA34Inputs {
    u8 pad000[0x84];
    void *data084;
    u8 pad088[0x110];
    void *data198;
    u8 *text19C;
    u8 *text1A0;
    u8 *text1A4;
    u8 *text1A8;
    u8 *text1AC;
} O101TailBA34Inputs;

extern O101TailBA34Root gO101TailBA34Root;
extern s32 gO101TailBA34OrderCount;
extern s32 gO101TailBA34OrderCountCall0;
extern s32 gO101TailBA34OrderCountCall1;
extern s32 gO101TailBA34OrderCountCall2;
extern void *gO101TailBA34OrderSlots[];
extern s32 gO101TailBA34Node32Count;
extern O101TailBA34Node32 gO101TailBA34Nodes32[];
extern s32 gO101TailBA34Node24Count;
extern O101TailBA34Node24 gO101TailBA34Nodes24[];
extern O101TailBA34Inputs gO101TailBA34Inputs;
extern u8 gO101TailBA34AssetDB8;
extern u8 gO101TailBA34FinalObject4630;

/* The three creator calls and finalizer are packet-local opaque identities. */
extern void *o101TailBA34RootCreatorReloc(s32 key, void *source,
                                          s32 *orderCount);
extern void *o101TailBA34ZeroCreatorReloc(s32 key, void *source,
                                          s32 *orderCount);
extern void *o101TailBA34NonzeroCreatorReloc(s32 key, void *source,
                                             s32 *orderCount);
extern void o101TailBA34FinalizerReloc(void *object);
extern s32 overlay101ByteLength(u8 *text);

/* Exact typed owner for overlay 101 +0xBA34..+0xC144. */
/* Workbench: structure-mismatch; size-exact, 393 words, first +0x0.
 * Levers: unifying the order counter changed size and worsened to 420 words.
 * Remains: the 0x60/0x40 frame and packet-local counter relocation aliases. */
#ifdef NON_MATCHING
void func_overlay_101_F000BA34_18E7254(s32 variant) {
    s32 orderIndex;
    s32 nodeIndex;
    void *handle;
    s32 length;
    O101TailBA34Node32 *node32;
    O101TailBA34Node24 *node24;
    register volatile f32 scale;

    orderIndex = gO101TailBA34OrderCountCall0;
    gO101TailBA34Root.height30 = 0xF0;
    gO101TailBA34Root.width2E = 0x140;
    gO101TailBA34Root.kind = 4;
    gO101TailBA34Root.asset34 = &gO101TailBA34AssetDB8;
    gO101TailBA34Root.color32 = 0xFF;
    gO101TailBA34Root.color33 = 0xFF;
    gO101TailBA34Root.value26 = 0;
    gO101TailBA34Root.value28 = 0;
    gO101TailBA34Root.value2A = 0;
    gO101TailBA34Root.value2C = 0;
    gO101TailBA34Root.chainType = 0;
    gO101TailBA34Root.chain = NULL;
    gO101TailBA34OrderSlots[orderIndex] = &gO101TailBA34Root.chainType;
    gO101TailBA34OrderCountCall0 = orderIndex + 1;

    nodeIndex = gO101TailBA34Node32Count;
    node32 = &gO101TailBA34Nodes32[nodeIndex];
    node32->x = 0x4E;
    node32->y = 0x14E;
    node32->value10 = 0;
    node32->color12 = 0xFF;
    node32->color13 = 0;
    node32->value18 = 0;
    node32->scale = 1.0f;
    node32->value14 = 0.0f;
    handle = o101TailBA34RootCreatorReloc(
        0x92, NULL, &gO101TailBA34OrderCountCall0);

    nodeIndex = gO101TailBA34Node32Count;
    node32 = &gO101TailBA34Nodes32[nodeIndex];
    node32->previousType = gO101TailBA34Root.chainType;
    node32->previous = gO101TailBA34Root.chain;
    node32->handle = handle;
    gO101TailBA34Root.chainType = 2;
    gO101TailBA34Root.chain = node32;
    gO101TailBA34Node32Count = nodeIndex + 1;

    orderIndex = gO101TailBA34OrderCountCall1;
    gO101TailBA34Root.x42 = 0x20;
    gO101TailBA34Root.width44 = 0x18;
    gO101TailBA34Root.y46 = 0x8C;
    gO101TailBA34Root.height48 = 0x18;
    gO101TailBA34Root.value4A = 0x5B;
    gO101TailBA34Root.value4C = 0x63;
    gO101TailBA34Root.mode40 = 0;
    gO101TailBA34Root.color4E = 0xFF;
    gO101TailBA34Root.color4F = 0xFF;
    gO101TailBA34Root.childType = 0;
    gO101TailBA34Root.child = NULL;
    gO101TailBA34Root.data50 = gO101TailBA34Inputs.data198;
    gO101TailBA34OrderSlots[orderIndex] = &gO101TailBA34Root.childType;
    gO101TailBA34OrderCountCall1 = orderIndex + 1;

    if (variant == 0) {
        nodeIndex = gO101TailBA34Node32Count;
        node32 = &gO101TailBA34Nodes32[nodeIndex];
        node32->x = 6;
        node32->y = 12;
        node32->scale = 1.0f;
        node32->value10 = 0;
        node32->color12 = 0xFF;
        node32->color13 = 0;
        node32->value14 = 0.0f;
        node32->value18 = 0;
        handle = o101TailBA34ZeroCreatorReloc(
            0xA0, NULL, &gO101TailBA34OrderCountCall1);
        nodeIndex = gO101TailBA34Node32Count;
        node32 = &gO101TailBA34Nodes32[nodeIndex];
        node32->previousType = gO101TailBA34Root.childType;
        node32->previous = gO101TailBA34Root.child;
        node32->handle = handle;
        gO101TailBA34Root.childType = 2;
        gO101TailBA34Root.child = node32;
        gO101TailBA34Node32Count = nodeIndex + 1;
    } else {
        nodeIndex = gO101TailBA34Node32Count;
        node32 = &gO101TailBA34Nodes32[nodeIndex];
        node32->x = 6;
        node32->y = 12;
        node32->scale = 1.0f;
        node32->value10 = 0;
        node32->color12 = 0xFF;
        node32->color13 = 0;
        node32->value14 = 0.0f;
        node32->value18 = 0;
        handle = o101TailBA34NonzeroCreatorReloc(
            0x9F, NULL, &gO101TailBA34OrderCountCall1);
        nodeIndex = gO101TailBA34Node32Count;
        node32 = &gO101TailBA34Nodes32[nodeIndex];
        node32->previousType = gO101TailBA34Root.childType;
        node32->previous = gO101TailBA34Root.child;
        node32->handle = handle;
        gO101TailBA34Root.childType = 2;
        gO101TailBA34Root.child = node32;
        gO101TailBA34Node32Count = nodeIndex + 1;
    }

    orderIndex = gO101TailBA34OrderCountCall1;
    gO101TailBA34Root.x5E = 0x20;
    gO101TailBA34Root.width60 = 0x40;
    gO101TailBA34Root.y62 = 0x5A;
    gO101TailBA34Root.height64 = 0x7E;
    gO101TailBA34Root.value66 = 0xCC;
    gO101TailBA34Root.value68 = 0x4A;
    gO101TailBA34Root.mode5C = 0;
    gO101TailBA34Root.color6A = 0xFF;
    gO101TailBA34Root.color6B = 0xFF;
    gO101TailBA34Root.tertiaryType = 0;
    gO101TailBA34Root.tertiary = NULL;
    gO101TailBA34Root.data6C = gO101TailBA34Inputs.data084;
    gO101TailBA34OrderSlots[orderIndex] = &gO101TailBA34Root.tertiaryType;
    gO101TailBA34OrderCountCall1 = orderIndex + 1;

    scale = 1.0f;

#define BUILD_BA34_TEXT(field, ypos)                                        \
    nodeIndex = gO101TailBA34Node24Count;                                   \
    node24 = &gO101TailBA34Nodes24[nodeIndex];                              \
    node24->x = 0x66;                                                       \
    node24->y = (ypos);                                                     \
    length = overlay101ByteLength(gO101TailBA34Inputs.field);               \
    nodeIndex = gO101TailBA34Node24Count;                                   \
    node24 = &gO101TailBA34Nodes24[nodeIndex];                              \
    node24->length = (u8)length;                                            \
    node24->opacity = (s8)(s32)((f32)(u32)(length & 0xFF) * scale);         \
    node24->mode = 2;                                                       \
    node24->color0 = 0;                                                     \
    node24->color1 = 0;                                                     \
    node24->color2 = 0;                                                     \
    node24->color3 = 0;                                                     \
    node24->kind = 4;                                                       \
    node24->text = gO101TailBA34Inputs.field;                               \
    node24->previousType = gO101TailBA34Root.tertiaryType;                  \
    node24->previous = gO101TailBA34Root.tertiary;                          \
    gO101TailBA34Root.tertiaryType = 3;                                     \
    gO101TailBA34Root.tertiary = node24;                                    \
    gO101TailBA34Node24Count = nodeIndex + 1

    BUILD_BA34_TEXT(text19C, 0x10);
    BUILD_BA34_TEXT(text1A0, 0x1E);
    BUILD_BA34_TEXT(text1A4, 0x28);
    BUILD_BA34_TEXT(text1A8, 0x32);
    BUILD_BA34_TEXT(text1AC, 0x3C);

#undef BUILD_BA34_TEXT

    o101TailBA34FinalizerReloc(&gO101TailBA34FinalObject4630);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o101/overlay101TailBA34/func_overlay_101_F000BA34_18E7254.s")
#endif
