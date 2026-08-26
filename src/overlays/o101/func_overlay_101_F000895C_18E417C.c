#include "PR/ultratypes.h"

typedef struct Root {
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
} Root;

typedef struct Node32 {
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
} Node32;

typedef struct Node24 {
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
    void *text;
} Node24;

extern Root D_0;
extern void *D_1C;
extern void *D_38;
extern u32 D_F4;
extern void *D_F8;
extern void *D_114;
extern void *D_118;
extern void *D_11C;
extern void *D_D04;
extern void *D_2C60;
extern void *D_1C0[];
extern s32 D_1C4;
extern s32 D_1CC;
extern s32 D_1D0;
extern Node32 D_340[];
extern Node24 D_540[];

extern void *func_overlay_101_F0000000_18DB820();
extern s8 func_overlay_101_F000CEA8_18E86C8(void *);

/* Workbench p4: structure-mismatch; 524/525 candidate/target instructions, 461 words from +0x4, frame -64.
 * Lever: retained-node/local-lifetime forms were checked against the target saved-register map; no exact source form emerged.
 * Remains: saved-node web and root/node relocation schedule. */
#ifdef NON_MATCHING
void func_overlay_101_F000895C_18E417C(void) {
    register volatile Node32 *node32;
    Node24 *node24;
    void *handle;
    s32 *ownerCount;
    s32 length;
    s32 nodeIndex;
    s32 ownerIndex;
    s32 textIndex;

    D_0.height30 = 0xF0;
    D_0.width2E = 0x140;
    D_0.kind = 4;
    D_0.asset34 = &D_D04;
    D_0.color32 = 0xFF;
    D_0.color33 = 0xFF;
    D_0.value26 = 0;
    D_0.value28 = 0;
    D_0.value2A = 0;
    D_0.value2C = 0;
    D_0.chainType = 0;
    D_0.chain = 0;
    ownerCount = &D_1C4;
    ownerIndex = *ownerCount;
    D_1C0[ownerIndex] = &D_1C;
    *ownerCount = ownerIndex + 1;

    node32 = &D_340[D_1CC];
    node32->x = 0x4E;
    node32->y = 0x14E;
    node32->value10 = 0;
    node32->color12 = 0xFF;
    node32->color13 = 0;
    node32->value18 = 0;
    node32->scale = 1.0f;
    node32->value14 = 0.0f;
    handle = func_overlay_101_F0000000_18DB820(0x92, 0);
    nodeIndex = D_1CC;
    node32 = &D_340[nodeIndex];
    node32->previousType = D_0.chainType;
    node32->previous = D_0.chain;
    node32->handle = handle;
    D_0.chainType = 2;
    D_0.chain = (void *)node32;
    D_1CC = nodeIndex + 1;

    D_0.x42 = 0x20;
    D_0.width44 = 0x18;
    D_0.y46 = 0x20;
    D_0.height48 = 0x14;
    D_0.value4A = 0x100;
    D_0.value4C = 0xB4;
    D_0.mode40 = 0;
    D_0.color4E = 0xFF;
    D_0.color4F = 0xFF;
    D_0.childType = 0;
    D_0.child = 0;
    D_0.data50 = D_F8;
    ownerCount = &D_1C4;
    ownerIndex = *ownerCount;
    D_1C0[ownerIndex] = &D_38;
    *ownerCount = ownerIndex + 1;

#define ADD_IMAGE_NODE(nodeX, nodeY, imageId)                                \
    node32 = &D_340[D_1CC];                                                  \
    node32->x = (nodeX);                                                     \
    node32->y = (nodeY);                                                     \
    node32->scale = 1.0f;                                                    \
    node32->value10 = 0;                                                     \
    node32->color12 = 0xFF;                                                  \
    node32->color13 = 0;                                                     \
    node32->value14 = 0.0f;                                                  \
    node32->value18 = 0;                                                     \
    handle = func_overlay_101_F0000000_18DB820((imageId), 0);                 \
    nodeIndex = D_1CC;                                                       \
    node32 = &D_340[nodeIndex];                                              \
    node32->handle = handle;                                                 \
    node32->previousType = D_0.childType;                                    \
    node32->previous = D_0.child;                                            \
    D_0.childType = 2;                                                       \
    D_0.child = (void *)node32;                                              \
    D_1CC = nodeIndex + 1

    if (((D_F4 << 5) >> 28) & 1) {
        ADD_IMAGE_NODE(0x98, 0x2E, 0x97);
    } else {
        ADD_IMAGE_NODE(0x98, 0x2E, 0x98);
    }
    if (((D_F4 << 5) >> 28) & 2) {
        ADD_IMAGE_NODE(0xDA, 0x2E, 0x99);
    } else {
        ADD_IMAGE_NODE(0xDA, 0x2E, 0x9A);
    }
    if (((D_F4 << 5) >> 28) & 4) {
        ADD_IMAGE_NODE(0x98, 0x70, 0x9D);
    } else {
        ADD_IMAGE_NODE(0x98, 0x70, 0x9E);
    }
    if (((D_F4 << 5) >> 28) & 8) {
        ADD_IMAGE_NODE(0xDA, 0x70, 0x9B);
    } else {
        ADD_IMAGE_NODE(0xDA, 0x70, 0x9C);
    }

#undef ADD_IMAGE_NODE

#define ADD_TEXT_ROW(field, rowY)                                            \
    textIndex = D_1D0;                                                       \
    node24 = &D_540[textIndex];                                              \
    node24->x = 0x80;                                                        \
    node24->y = (rowY);                                                      \
    length = func_overlay_101_F000CEA8_18E86C8(field);                       \
    textIndex = D_1D0;                                                       \
    node24 = &D_540[textIndex];                                              \
    node24->length = (u8)length;                                             \
    node24->opacity =                                                        \
        (s8)(s32)((f32)(u32)(length & 0xFF) * (f32)(s32)1);                  \
    node24->mode = 2;                                                        \
    node24->color0 = 0;                                                      \
    node24->color1 = 0;                                                      \
    node24->color2 = 0;                                                      \
    node24->color3 = 0;                                                      \
    node24->kind = 4;                                                        \
    node24->text = (field);                                                  \
    node24->previousType = D_0.childType;                                    \
    node24->previous = D_0.child;                                            \
    D_0.childType = 3;                                                       \
    D_0.child = node24;                                                      \
    D_1D0 = textIndex + 1

    ADD_TEXT_ROW(D_114, 0x92);
    ADD_TEXT_ROW(D_118, 0x9C);
    ADD_TEXT_ROW(D_11C, 0xA6);

#undef ADD_TEXT_ROW

    func_overlay_101_F0000000_18DB820(&D_2C60);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o101/func_overlay_101_F000895C_18E417C/func_overlay_101_F000895C_18E417C.s")
#endif
