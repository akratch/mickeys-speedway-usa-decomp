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
    s32 secondChildType;
    void *secondChild;
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
extern void *D_54;
extern void *D_80;
extern void *D_84;
extern void *D_88;
extern void *D_8C;
extern void *D_90;
extern void *D_94;
extern void *D_C8C;
extern void *D_10EC;
extern void *D_1C0[];
extern s32 D_1C4;
extern s32 D_1CC;
extern s32 D_1D0;
extern f32 D_EA0;
extern f32 D_EA4;
extern f32 D_EA8;
extern Node32 D_340[];
extern Node24 D_540[];

extern void *func_overlay_101_F0000000_18DB820();
extern s8 func_overlay_101_F000CEA8_18E86C8(void *);

/* Mickey-local structural sibling: func_overlay_101_F00063F8_18E1C18. */
/* Workbench: structure-mismatch; size-exact, 326 words, first +0x2C.
 * Levers: counter-index temporaries and store-order audit worsened register webs.
 * Remains: local-overlay relocation identity and the root-store schedule. */
#ifdef NON_MATCHING
void func_overlay_101_F000571C_18E0F3C(void) {
    s32 index;
    s32 length;
    void *handle;
    Node24 *node24;
    Node32 *node32;

    D_0.kind = 4;
    D_0.width2E = 0x140;
    D_0.height30 = 0xF0;
    D_0.asset34 = &D_C8C;
    D_0.color32 = 0xFF;
    D_0.color33 = 0xFF;
    D_0.value26 = 0;
    D_0.value28 = 0;
    D_0.value2A = 0;
    D_0.value2C = 0;
    D_0.chainType = 0;
    D_0.chain = 0;
    D_1C0[D_1C4] = &D_1C;
    D_1C4 += 1;

    index = D_1CC;
    node32 = &D_340[index];
    node32->x = 0x4E;
    node32->y = 0x14E;
    node32->value10 = 0;
    node32->color12 = 0xFF;
    node32->color13 = 0;
    node32->value18 = 0;
    node32->scale = 1.0f;
    node32->value14 = 0.0f;
    handle = func_overlay_101_F0000000_18DB820(0x92, 0);
    index = D_1CC;
    node32 = &D_340[index];
    node32->previousType = D_0.chainType;
    node32->previous = D_0.chain;
    node32->handle = handle;
    D_0.chainType = 2;
    D_0.chain = node32;
    D_1CC = index + 1;

    D_0.x42 = 0x20;
    D_0.width44 = 0x18;
    D_0.y46 = 0x5A;
    D_0.height48 = 0x20;
    D_0.value4A = 0xCC;
    D_0.value4C = 0x54;
    D_0.mode40 = 0;
    D_0.color4E = 0xFF;
    D_0.color4F = 0xFF;
    D_0.childType = 0;
    D_0.child = 0;
    D_0.data50 = D_80;
    D_1C0[D_1C4] = &D_38;
    D_1C4 += 1;

#define ADD_IMAGE_NODE(nodeX, imageScale, imageId)                            \
    index = D_1CC;                                                           \
    node32 = &D_340[index];                                                  \
    node32->x = (nodeX);                                                     \
    node32->y = 0xE;                                                        \
    node32->value10 = 0;                                                     \
    node32->color12 = 0xFF;                                                  \
    node32->color13 = 0;                                                     \
    node32->value18 = 0;                                                     \
    node32->scale = (imageScale);                                            \
    node32->value14 = 0.0f;                                                  \
    handle = func_overlay_101_F0000000_18DB820((imageId), 0);                 \
    index = D_1CC;                                                           \
    node32 = &D_340[index];                                                  \
    node32->handle = handle;                                                 \
    node32->previousType = D_0.childType;                                    \
    node32->previous = D_0.child;                                            \
    D_0.childType = 2;                                                       \
    D_0.child = node32;                                                      \
    D_1CC = index + 1

    ADD_IMAGE_NODE(6, D_EA0, 0xA1);
    ADD_IMAGE_NODE(0x46, D_EA4, 0xA0);
    ADD_IMAGE_NODE(0x86, D_EA8, 0xA0);

#undef ADD_IMAGE_NODE

    D_0.x5E = 0x20;
    D_0.width60 = 0x40;
    D_0.y62 = 0x60;
    D_0.height64 = 0x78;
    D_0.value66 = 0xC0;
    D_0.value68 = 0x46;
    D_0.mode5C = 0;
    D_0.color6A = 0xFF;
    D_0.color6B = 0xFF;
    D_0.secondChildType = 0;
    D_0.secondChild = 0;
    D_0.data6C = D_84;
    D_1C0[D_1C4] = &D_54;
    D_1C4 += 1;

#define ADD_TEXT_ROW(field, rowY)                                            \
    index = D_1D0;                                                           \
    node24 = &D_540[index];                                                  \
    node24->x = 0x60;                                                        \
    node24->y = (rowY);                                                      \
    length = func_overlay_101_F000CEA8_18E86C8(field);                       \
    index = D_1D0;                                                           \
    node24 = &D_540[index];                                                  \
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
    node24->previousType = D_0.secondChildType;                              \
    node24->previous = D_0.secondChild;                                      \
    D_0.secondChildType = 3;                                                 \
    D_0.secondChild = node24;                                                \
    D_1D0 = index + 1

    ADD_TEXT_ROW(D_88, 0x10);
    ADD_TEXT_ROW(D_8C, 0x1E);
    ADD_TEXT_ROW(D_90, 0x28);
    ADD_TEXT_ROW(D_94, 0x36);

#undef ADD_TEXT_ROW

    func_overlay_101_F0000000_18DB820(&D_10EC);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o101/func_overlay_101_F000571C_18E0F3C/func_overlay_101_F000571C_18E0F3C.s")
#endif
