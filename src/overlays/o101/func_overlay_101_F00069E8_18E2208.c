#include "PR/ultratypes.h"

typedef struct O101RootGroup {
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
} O101RootGroup;

typedef struct O101Root69E8 {
    u8 pad00[0x1C];
    O101RootGroup groups[6];
} O101Root69E8;

typedef struct O101Node32_69E8 {
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
} O101Node32_69E8;

typedef struct O101Node20_69E8 {
    s32 previousType;
    void *previous;
    s16 x;
    s16 y;
    f32 scale;
    void *handle;
} O101Node20_69E8;

typedef struct O101Node24_69E8 {
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
} O101Node24_69E8;

typedef struct O101Inputs69E8 {
    u8 pad000[0xC8];
    void *dataC8;
    void *textCC;
    void *textD0;
    void *dataD4;
    void *dataD8;
    void *textDC;
    void *textE0;
    void *dataE4;
    void *textE8;
    void *textEC;
    void *dataF0;
    void *textF4;
} O101Inputs69E8;

extern O101Root69E8 D_0;
extern O101Inputs69E8 D_INPUT;
extern void *D_1C;
extern void *D_38;
extern void *D_54;
extern void *D_70;
extern void *D_8C;
extern void *D_A8;
extern void *D_CC8;
extern void *D_1D2C;
extern void *D_1C0[];
extern s32 D_1C4;
extern s32 D_1C8;
extern s32 D_1CC;
extern s32 D_1D0;
extern O101Node20_69E8 D_200[];
extern O101Node32_69E8 D_340[];
extern O101Node24_69E8 D_540[];

extern void *func_overlay_101_F0000000_18DB820();
extern s8 func_overlay_101_F000CEA8_18E86C8(void *);

/*
 * Mickey-local reconstruction from this overlay's extracted function and
 * the typed node builders immediately before and after it. The pinned DKR
 * v77/v80 and JFG overlay scans report no donor for overlay 101.
 * Workbench: structure-mismatch; 840/963 masked positional words differ, first +0x2C;
 * 963/967 instructions and an exact -0x50 frame on target and candidate.
 * Levers: MIPS-II flags, target-derived node/base/owner-index and root-store probes; remaining drift is saved-register/store scheduling across builders.
 */
#ifdef NON_MATCHING
void func_overlay_101_F00069E8_18E2208(void) {
    register s32 commonY;
    s32 index;
    s32 orderIndex;
    s32 length;
    void *handle;
    O101Node20_69E8 *node20;
    O101Node24_69E8 *node24;
    O101Node32_69E8 *node32;

#define INIT_GROUP(group, px, pwidth, py, pheight, value0, value1, input,    \
                   owner)                                                    \
    D_0.groups[group].x = (px);                                              \
    D_0.groups[group].width = (pwidth);                                      \
    D_0.groups[group].y = (py);                                              \
    D_0.groups[group].height = (pheight);                                    \
    D_0.groups[group].value12 = (value0);                                    \
    D_0.groups[group].value14 = (value1);                                    \
    D_0.groups[group].mode = 0;                                              \
    D_0.groups[group].color16 = 0xFF;                                        \
    D_0.groups[group].color17 = 0xFF;                                        \
    D_0.groups[group].childType = 0;                                         \
    D_0.groups[group].child = 0;                                             \
    D_0.groups[group].data18 = D_INPUT.input;                                \
    orderIndex = D_1C4;                                                      \
    D_1C0[orderIndex] = &(owner);                                            \
    D_1C4 = orderIndex + 1

#define ADD_NODE32(group, nodeX, nodeY, nodeScale, nodeValue, nodeColor,    \
                   imageId)                                                  \
    index = D_1CC;                                                           \
    node32 = &D_340[index];                                                  \
    node32->x = (nodeX);                                                     \
    node32->y = (nodeY);                                                     \
    node32->value10 = (nodeValue);                                           \
    node32->color12 = (nodeColor);                                           \
    node32->color13 = 0;                                                     \
    node32->value18 = 0;                                                     \
    node32->scale = (nodeScale);                                             \
    node32->value14 = 0.0f;                                                  \
    handle = func_overlay_101_F0000000_18DB820((imageId), 0);                \
    index = D_1CC;                                                           \
    node32 = &D_340[index];                                                  \
    node32->previousType = D_0.groups[group].childType;                      \
    node32->previous = D_0.groups[group].child;                              \
    node32->handle = handle;                                                 \
    D_0.groups[group].childType = 2;                                         \
    D_0.groups[group].child = node32;                                        \
    D_1CC = index + 1

#define ADD_NODE20(group, nodeX, nodeY, imageId)                             \
    index = D_1C8;                                                           \
    node20 = &D_200[index];                                                  \
    node20->x = (nodeX);                                                     \
    node20->y = (nodeY);                                                     \
    node20->scale = 1.0f;                                                    \
    handle = func_overlay_101_F0000000_18DB820((imageId), 0);                \
    index = D_1C8;                                                           \
    node20 = &D_200[index];                                                  \
    node20->previousType = D_0.groups[group].childType;                      \
    node20->previous = D_0.groups[group].child;                              \
    node20->handle = handle;                                                 \
    D_0.groups[group].childType = 1;                                         \
    D_0.groups[group].child = node20;                                        \
    D_1C8 = index + 1

#define ADD_TEXT(group, input, textX, textY)                                 \
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
    node24->color0 = 0xFF;                                                   \
    node24->color1 = 0xFF;                                                   \
    node24->color2 = 0xFF;                                                   \
    node24->color3 = 0xFF;                                                   \
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
    D_0.groups[0].data18 = &D_CC8;
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

    ADD_NODE32(0, 0xF2, 0x14E, 1.0f, 0, 0xFF, 0x91);
    commonY = 0x78;
    ADD_NODE32(0, 0xA0, commonY, 5.0f, 0xE38, 0, 0x96);
    ADD_NODE32(0, 0xA0, commonY, 5.0f, 0x71C, 0, 0x96);
    ADD_NODE32(0, 0xA0, commonY, 5.0f, 0, 0, 0x96);
    ADD_NODE32(0, 0xA0, commonY, 5.0f, 0xE38, 0, 0x95);
    ADD_NODE32(0, 0xA0, commonY, 5.0f, 0x71C, 0, 0x95);
    ADD_NODE32(0, 0xA0, commonY, 5.0f, 0, 0, 0x95);
    ADD_NODE32(0, 0xA0, commonY, 5.0f, 0xE38, 0, 0x94);
    ADD_NODE32(0, 0xA0, commonY, 5.0f, 0x71C, 0, 0x94);
    ADD_NODE32(0, 0xA0, commonY, 5.0f, 0, 0, 0x94);

    INIT_GROUP(1, 0x20, 0x18, 0x30, 0x10, 0xE0, 0xC0, dataC8, D_38);
    ADD_NODE20(1, 0xC, 0xE, 0xD);
    ADD_TEXT(1, textCC, 0x70, 0xA8);
    ADD_TEXT(1, textD0, 0x70, 0xB2);

    INIT_GROUP(2, 0x20, 0x40, 0x62, 0x18, 0x7C, 0x84, dataD4, D_54);
    ADD_NODE20(2, 0x16, 0x1E, 0x10);

    INIT_GROUP(3, 0x20, 0x68, 0x40, 0xA0, 0xC0, 0x26, dataD8, D_70);
    ADD_TEXT(3, textDC, 0x60, 0xE);
    ADD_TEXT(3, textE0, 0x60, 0x18);

    INIT_GROUP(4, 0x20, 0x90, 0x2F, 0x10, 0xE2, 0xC0, dataE4, D_8C);
    ADD_NODE20(4, 0xD, 0xE, 0x14);
    ADD_TEXT(4, textE8, 0x71, 0xA8);
    ADD_TEXT(4, textEC, 0x71, 0xB2);

    INIT_GROUP(5, 0x90, -0x50, 0x30, 0x30, 0xE0, 0x8C, dataF0, D_A8);
    ADD_NODE20(5, 0xC, 0x12, 0xC);
    ADD_TEXT(5, textF4, 0x70, 0x7B);

    func_overlay_101_F0000000_18DB820(&D_1D2C);

#undef ADD_TEXT
#undef ADD_NODE20
#undef ADD_NODE32
#undef INIT_GROUP
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o101/func_overlay_101_F00069E8_18E2208/func_overlay_101_F00069E8_18E2208.s")
#endif
