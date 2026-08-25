#include "PR/ultratypes.h"

typedef struct O101RootGroup3A58 {
    s32 childType;
    void *child;
    u8 mode;
    u8 pad09;
    s16 x;
    s16 width;
    s16 y;
    s16 height;
    s16 value12;
    s16 value14;
    u8 color16;
    u8 color17;
    void *data18;
} O101RootGroup3A58;

typedef struct O101Root3A58 {
    u8 pad00[0x1C];
    O101RootGroup3A58 groups[11];
} O101Root3A58;

typedef struct O101Node32_3A58 {
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
} O101Node32_3A58;

typedef struct O101Node20_3A58 {
    s32 previousType;
    void *previous;
    s16 x;
    s16 y;
    f32 scale;
    void *handle;
} O101Node20_3A58;

typedef struct O101Node24_3A58 {
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
} O101Node24_3A58;

typedef struct O101Inputs3A58 {
    void *data0;
    void *text4;
    void *text8;
    void *dataC;
    void *text10;
    void *data14;
    void *data18;
    void *text1C;
    void *text20;
    void *text24;
    void *data28;
    void *text2C;
    void *data30;
    void *text34;
    void *data38;
    void *text3C;
    void *data40;
    void *text44;
    void *data48;
    void *text4C;
    void *text50;
    void *data54;
    void *text58;
    void *text5C;
    void *text60;
    void *text64;
} O101Inputs3A58;

extern O101Root3A58 D_0;
extern O101Inputs3A58 D_INPUT;
extern void *D_1C;
extern void *D_38;
extern void *D_54;
extern void *D_70;
extern void *D_8C;
extern void *D_A8;
extern void *D_C4;
extern void *D_E0;
extern void *D_FC;
extern void *D_118;
extern void *D_134;
extern void *D_C64;
extern void *D_1C0[];
extern s32 D_1C4;
extern s32 D_1C8;
extern s32 D_1CC;
extern s32 D_1D0;
extern O101Node20_3A58 D_200[];
extern u8 D_340[];
extern O101Node24_3A58 D_540[];

extern void *func_overlay_101_F0000000_18DB820();
extern s8 func_overlay_101_F000CEA8_18E86C8(void *);

/*
 * Mickey-local reconstruction from this overlay's extracted function and
 * its typed node-builder siblings. The pinned DKR v77/v80 and JFG overlay
 * scans report no donor for overlay 101.
 * NON_MATCHING p2: workbench structure-mismatch; best line-assignment variant
 * is 1,386/1,461 positional words, first +0x34, at 1,463/1,461 instructions.
 * Levers 1, 4, 23: constants/flags are exact; macro/register cascade remains.
 */
#ifdef NON_MATCHING
void func_overlay_101_F0003A58_18DF278(void) {
    s32 index;
    s32 orderIndex;
    s32 length;
    void *handle;
    O101Node20_3A58 *node20;
    O101Node24_3A58 *node24;
    O101Node32_3A58 *node32;

#define INIT_GROUP(group, px, pwidth, py, pheight, value0, value1, input,    \
                   owner)                                                    \
    D_0.groups[group].value14 = (value1);                                    \
    D_0.groups[group].value12 = (value0);                                    \
    D_0.groups[group].height = (pheight);                                    \
    D_0.groups[group].y = (py);                                              \
    D_0.groups[group].x = (px);                                              \
    D_0.groups[group].width = (pwidth);                                      \
    D_0.groups[group].mode = 0;                                              \
    D_0.groups[group].color16 = 0xFF;                                        \
    D_0.groups[group].color17 = 0xFF;                                        \
    D_0.groups[group].childType = 0;                                         \
    D_0.groups[group].child = 0;                                             \
    D_0.groups[group].data18 = D_INPUT.input;                                \
    orderIndex = D_1C4;                                                      \
    D_1C0[orderIndex] = &(owner);                                            \
    D_1C4 = orderIndex + 1

#define ADD_NODE32(group, nodeX, nodeY, imageId)                            \
    index = D_1CC;                                                           \
    node32 = (O101Node32_3A58 *)(D_340 + (index << 5));                       \
    node32->x = (nodeX);                                                     \
    node32->y = (nodeY);                                                     \
    node32->value10 = 0;                                                     \
    node32->color12 = 0xFF;                                                  \
    node32->color13 = 0;                                                     \
    node32->value18 = 0;                                                     \
    node32->scale = 1.0f;                                                    \
    node32->value14 = 0.0f;                                                  \
    handle = func_overlay_101_F0000000_18DB820((imageId), 0);                \
    index = D_1CC;                                                           \
    node32 = (O101Node32_3A58 *)(D_340 + (index << 5));                       \
    node32->previousType = D_0.groups[group].childType;                      \
    node32->previous = D_0.groups[group].child;                              \
    node32->handle = handle;                                                 \
    D_0.groups[group].childType = 2;                                         \
    D_0.groups[group].child = node32;                                        \
    D_1CC = index + 1

#define ADD_NODE20(group, nodeX, nodeY, imageId)                            \
    index = D_1C8;                                                           \
    node20 = &D_200[index];                                                  \
    node20->x = (nodeX);                                                     \
    node20->y = (nodeY);                                                     \
    node20->scale = 1.0f;                                                    \
    handle = func_overlay_101_F0000000_18DB820((imageId));                   \
    index = D_1C8;                                                           \
    node20 = &D_200[index];                                                  \
    node20->previousType = D_0.groups[group].childType;                      \
    node20->previous = D_0.groups[group].child;                              \
    node20->handle = handle;                                                 \
    D_0.groups[group].childType = 1;                                         \
    D_0.groups[group].child = node20;                                        \
    D_1C8 = index + 1

#define ADD_TEXT(group, input, textX, textY, c0, c1, c2, c3)                \
    index = D_1D0;                                                           \
    node24 = &D_540[index];                                                  \
    node24->x = (textX);                                                     \
    node24->y = (textY);                                                     \
    length = func_overlay_101_F000CEA8_18E86C8(D_INPUT.input);               \
    index = D_1D0;                                                           \
    node24 = &D_540[index];                                                  \
    node24->length = (u8)length;                                             \
    node24->opacity =                                                        \
        (s8)(s32)((f32)(u32)(length & 0xFF) * (f32)(s32)1);                  \
    node24->mode = 2;                                                        \
    node24->color0 = (c0);                                                   \
    node24->color1 = (c1);                                                   \
    node24->color2 = (c2);                                                   \
    node24->color3 = (c3);                                                   \
    node24->kind = 4;                                                        \
    node24->text = D_INPUT.input;                                            \
    node24->previousType = D_0.groups[group].childType;                      \
    node24->previous = D_0.groups[group].child;                              \
    D_0.groups[group].childType = 3;                                         \
    D_0.groups[group].child = node24;                                        \
    D_1D0 = index + 1

    D_0.groups[0].mode = 4;
    D_0.groups[0].value12 = 0x140;
    D_0.groups[0].value14 = 0xF0;
    D_0.groups[0].data18 = &D_C64;
    D_0.groups[0].color16 = 0xFF;
    D_0.groups[0].color17 = 0xFF;
    D_0.groups[0].x = 0;
    D_0.groups[0].width = 0;
    D_0.groups[0].y = 0;
    D_0.groups[0].height = 0;
    D_0.groups[0].childType = 0;
    D_0.groups[0].child = 0;
    orderIndex = D_1C4;
    D_1C0[orderIndex] = &D_1C;
    D_1C4 = orderIndex + 1;

    ADD_NODE32(0, 242, 334, 145);

    INIT_GROUP(1, 32, 24, 48, 36, 224, 168, data0, D_38);
    ADD_TEXT(1, text4, 112, 144, 255, 255, 255, 255);
    ADD_TEXT(1, text8, 112, 154, 255, 255, 255, 255);
    ADD_NODE20(1, -100, 14, 4);

    INIT_GROUP(2, 32, 104, 80, 57, 160, 126, dataC, D_54);
    ADD_TEXT(2, text10, 80, 112, 255, 255, 255, 255);
    ADD_NODE20(2, 32, 14, 11);

    INIT_GROUP(3, 32, 64, 58, 24, 204, 192, data14, D_70);
    ADD_NODE20(3, 6, 14, 6);

    INIT_GROUP(4, 40, -132, 40, -132, 240, 132, data18, D_8C);
    ADD_TEXT(4, text1C, 120, 96, 255, 255, 255, 255);
    ADD_TEXT(4, text20, 120, 106, 255, 255, 255, 255);
    ADD_TEXT(4, text24, 120, 116, 255, 255, 255, 255);
    ADD_NODE20(4, 80, 14, 14);

    INIT_GROUP(5, -64, -12, 64, 16, 96, 104, data28, D_A8);
    ADD_TEXT(5, text2C, 48, 90, 255, 192, 192, 255);
    ADD_NODE20(5, 8, 13, 3);

    INIT_GROUP(6, 320, -12, 160, 16, 96, 104, data30, D_C4);
    ADD_TEXT(6, text34, 48, 90, 192, 192, 255, 255);
    ADD_NODE20(6, 8, 13, 5);

    INIT_GROUP(7, 320, 240, 160, 120, 96, 104, data38, D_E0);
    ADD_TEXT(7, text3C, 48, 90, 255, 192, 192, 255);
    ADD_NODE20(7, 8, 13, 18);

    INIT_GROUP(8, -64, 240, 64, 120, 96, 104, data40, D_FC);
    ADD_TEXT(8, text44, 48, 90, 255, 255, 192, 255);
    ADD_NODE20(8, 8, 13, 8);

    INIT_GROUP(9, 76, -162, 76, -162, 172, 162, data48, D_118);
    ADD_TEXT(9, text4C, 86, 138, 192, 255, 192, 255);
    ADD_TEXT(9, text50, 86, 148, 192, 255, 192, 255);
    ADD_NODE20(9, 6, 14, 10);

    INIT_GROUP(10, 32, 144, 32, 91, 192, 62, data54, D_134);
    ADD_TEXT(10, text58, 96, 14, 0, 0, 0, 0);
    ADD_TEXT(10, text5C, 96, 24, 0, 0, 0, 0);
    ADD_TEXT(10, text60, 96, 34, 0, 0, 0, 0);
    ADD_TEXT(10, text64, 96, 48, 0, 0, 0, 0);

    func_overlay_101_F0000000_18DB820(D_340);

#undef ADD_TEXT
#undef ADD_NODE20
#undef ADD_NODE32
#undef INIT_GROUP
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o101/func_overlay_101_F0003A58_18DF278/func_overlay_101_F0003A58_18DF278.s")
#endif
