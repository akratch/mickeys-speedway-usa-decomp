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
    u8 pad54[0x130];
    void *text184;
    void *text188;
    void *text18C;
    void *text190;
    void *text194;
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

typedef struct Inputs {
    u8 pad000[0x184];
    void *data184;
    void *text188;
    void *text18C;
    void *text190;
    void *text194;
} Inputs;

extern Root D_0;
extern Inputs D_INPUT;
extern void *D_1C;
extern void *D_38;
extern void *D_DA4;
extern void *D_4358;
extern void *D_1C0[];
extern s32 D_1C4;
extern s32 D_1CC;
extern s32 D_1D0;
extern Node32 D_340[];
extern Node24 D_540[];

extern void *func_overlay_101_F0000000_18DB820();
extern s32 func_overlay_101_F000CEA8_18E86C8(void *);

/* PLATEAU (2026-08-25): best preserved permutation is exact-size at 316 words; 249 differ, first +0x10.
 * The retained unsigned-zero shape has 263 differences and fixes 0x38 frame; saved-temp scheduling remains.
 * Flag lattice, root-member modeling, and declaration/load ordering did not close it; no donor used. */
#ifdef NON_MATCHING
void func_overlay_101_F000B544_18E6D64(void) {
    s32 orderIndex;
    s32 node32Index;
    s32 node24Index;
    s32 length;
    void *handle;
    Node32 *node32;
    Node24 *node24;

    D_0.height30 = 0xF0;
    D_0.width2E = 0x140;
    D_0.kind = 4;
    D_0.asset34 = &D_DA4;
    D_0.color32 = 0xFF;
    D_0.color33 = 0xFF;
    D_0.value26 = 0;
    D_0.value28 = 0;
    D_0.value2A = 0;
    D_0.value2C = 0;
    D_0.chainType = 0;
    D_0.chain = 0;
    orderIndex = D_1C4;
    D_1C0[orderIndex] = &D_1C;
    D_1C4 = orderIndex + 1;

    node32Index = D_1CC;
    node32 = &D_340[node32Index];
    node32->x = 0x108;
    node32->y = 0x154;
    node32->value10 = 0;
    node32->color12 = 0xFF;
    node32->color13 = 0;
    node32->value18 = 0;
    node32->scale = 1.0f;
    node32->value14 = 0.0f;
    handle = func_overlay_101_F0000000_18DB820(0x91, 0);
    node32Index = D_1CC;
    node32 = &D_340[node32Index];
    node32->previousType = D_0.chainType;
    node32->previous = D_0.chain;
    node32->handle = handle;
    D_0.chainType = 2;
    D_0.chain = node32;
    D_1CC = node32Index + 1;

    orderIndex = D_1C4;
    D_0.x42 = 0x20;
    D_0.width44 = 0x18;
    D_0.y46 = 0x1C;
    D_0.height48 = 0x50;
    D_0.value4A = 0xD8;
    D_0.value4C = 0x42;
    D_0.mode40 = 0;
    D_0.color4E = 0xFF;
    D_0.color4F = 0xFF;
    D_0.childType = 0;
    D_0.child = 0;
    D_0.data50 = D_INPUT.data184;
    D_1C0[orderIndex] = &D_38;
    D_1C4 = orderIndex + 1;

    node24Index = D_1D0;
    node24 = &D_540[node24Index];
    node24->x = 0x6C;
    node24->y = 0x10;
    length = func_overlay_101_F000CEA8_18E86C8(D_INPUT.text188);
    node24Index = D_1D0;
    node24 = &D_540[node24Index];
    node24->length = (u8)length;
    node24->opacity =
        (s8)(s32)((f32)(u32)(length & 0xFF) * (f32)(u32)0);
    node24->mode = 2;
    node24->color0 = 0xFF;
    node24->color1 = 0x80;
    node24->color2 = 0;
    node24->color3 = 0xFF;
    node24->kind = 4;
    node24->text = D_INPUT.text188;
    node24->previousType = D_0.childType;
    node24->previous = D_0.child;
    D_0.childType = 3;
    D_0.child = node24;
    D_1D0 = node24Index + 1;

    node24Index = D_1D0;
    node24 = &D_540[node24Index];
    node24->x = 0x6C;
    node24->y = 0x1E;
    length = func_overlay_101_F000CEA8_18E86C8(D_INPUT.text18C);
    node24Index = D_1D0;
    node24 = &D_540[node24Index];
    node24->length = (u8)length;
    node24->opacity =
        (s8)(s32)((f32)(u32)(length & 0xFF) * (f32)(u32)0);
    node24->mode = 2;
    node24->color0 = 0xFF;
    node24->color1 = 0xFF;
    node24->color2 = 0;
    node24->color3 = 0xFF;
    node24->kind = 4;
    node24->text = D_INPUT.text18C;
    node24->previousType = D_0.childType;
    node24->previous = D_0.child;
    D_0.childType = 3;
    D_0.child = node24;
    D_1D0 = node24Index + 1;

    node24Index = D_1D0;
    node24 = &D_540[node24Index];
    node24->x = 0x6C;
    node24->y = 0x28;
    length = func_overlay_101_F000CEA8_18E86C8(D_INPUT.text190);
    node24Index = D_1D0;
    node24 = &D_540[node24Index];
    node24->length = (u8)length;
    node24->opacity =
        (s8)(s32)((f32)(u32)(length & 0xFF) * (f32)(u32)0);
    node24->mode = 2;
    node24->color0 = 0xFF;
    node24->color1 = 0xFF;
    node24->color2 = 0;
    node24->color3 = 0xFF;
    node24->kind = 4;
    node24->text = D_INPUT.text190;
    node24->previousType = D_0.childType;
    node24->previous = D_0.child;
    D_0.childType = 3;
    D_0.child = node24;
    D_1D0 = node24Index + 1;

    node24Index = D_1D0;
    node24 = &D_540[node24Index];
    node24->x = 0x6C;
    node24->y = 0x32;
    length = func_overlay_101_F000CEA8_18E86C8(D_INPUT.text194);
    node24Index = D_1D0;
    node24 = &D_540[node24Index];
    node24->length = (u8)length;
    node24->opacity =
        (s8)(s32)((f32)(u32)(length & 0xFF) * (f32)(u32)0);
    node24->mode = 2;
    node24->color0 = 0xFF;
    node24->color1 = 0xFF;
    node24->color2 = 0;
    node24->color3 = 0xFF;
    node24->kind = 4;
    node24->text = D_INPUT.text194;
    node24->previousType = D_0.childType;
    node24->previous = D_0.child;
    D_0.childType = 3;
    D_0.child = node24;
    D_1D0 = node24Index + 1;

    func_overlay_101_F0000000_18DB820(&D_4358);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o101/overlay101TailB544/func_overlay_101_F000B544_18E6D64.s")
#endif
