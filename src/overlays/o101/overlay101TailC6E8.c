#include "PR/ultratypes.h"

typedef struct O101TailC6E8Root {
    s32 chainType;
    void *chain;
    u8 kind;
    u8 pad09;
    s16 value0A;
    s16 value0C;
    s16 x;
    s16 y;
    s16 width;
    s16 height;
    u8 color0;
    u8 color1;
    void *asset;
} O101TailC6E8Root;

typedef struct O101TailC6E8Node32 {
    s32 previousType;
    void *previous;
    s16 x;
    s16 y;
    f32 scale;
    s16 value10;
    u8 color0;
    u8 color1;
    f32 value14;
    s32 value18;
    void *handle;
} O101TailC6E8Node32;

typedef struct O101TailC6E8Node20 {
    s32 previousType;
    void *previous;
    s16 x;
    s16 y;
    f32 scale;
    void *handle;
} O101TailC6E8Node20;

extern s32 gO101TailC6E8QueueCount;
extern u8 gO101TailC6E8Selector;
extern u8 gO101TailC6E8QueueBytes[];
extern s32 gO101TailC6E8PresentationActive;
extern s32 gO101TailC6E8PresentationDone;
extern O101TailC6E8Root gO101TailC6E8Root;
extern s32 gO101TailC6E8OrderCount;
extern O101TailC6E8Root *gO101TailC6E8OrderSlots[];
extern s32 gO101TailC6E8Node32Count;
extern O101TailC6E8Node32 gO101TailC6E8Nodes32[];
extern s32 gO101TailC6E8Node20Count;
extern O101TailC6E8Node20 gO101TailC6E8Nodes20[];
extern u8 gO101TailC6E8AssetDF0;
extern void *gO101TailC6E8Handle1D4;
extern void *gO101TailC6E8Handle1F4;
extern void *gO101TailC6E8Handle33C;
extern void *gO101TailC6E8Handle338;

extern s32 o101TailC6E8StateReloc(void);
extern void o101TailC6E8SpecialReloc();
extern void o101TailC6E8Block00Reloc();
extern void o101TailC6E8Block01Reloc();
extern void o101TailC6E8Block02Reloc();
extern void o101TailC6E8Block03Reloc();
extern void o101TailC6E8Block04Reloc();
extern void o101TailC6E8Block05Reloc();
extern void o101TailC6E8Block06Reloc();
extern void o101TailC6E8Block07Reloc();
extern void o101TailC6E8Block08Reloc();
extern void o101TailC6E8Block09Reloc();
extern void overlay101BuildPresentationA(void);
extern void o101TailC6E8Block11Reloc();
extern void o101TailC6E8Block12Reloc();
extern void o101TailC6E8Block13Reloc();
extern void o101TailC6E8Block14Reloc();
extern void o101TailC6E8Block15Reloc();
extern void o101TailC6E8Block16Reloc();
extern void o101TailC6E8Block17Reloc();
extern void o101TailC6E8Block18Reloc();
extern void o101TailC6E8Block19Reloc();
extern void o101TailC6E8Block20Reloc();
extern void o101TailC6E8Block21Reloc();
extern void o101TailC6E8Block22Reloc();
extern void o101TailC6E8Block23Reloc();
extern void o101TailC6E8Block24Reloc();
extern void o101TailC6E8DefaultReloc();
extern void *o101TailC6E8Create8FReloc();
extern void *o101TailC6E8Create90Reloc();
extern void *o101TailC6E8CreateCompactReloc();
extern void *o101TailC6E8Acquire5F1Reloc();
extern void *o101TailC6E8Acquire5F2Reloc();
extern void *o101TailC6E8Acquire58Reloc();
extern void *o101TailC6E8Acquire338Reloc();
extern void o101TailC6E8Tail27Reloc();
extern void o101TailC6E8TailFinalReloc();

/*
 * The retained 25-entry table at overlay-local +0xEAC maps selector values
 * 0..24 to these lexical cases in order. Atlas runtime records preserve the
 * distinct opaque call identities even though the split object uses one raw
 * static carrier for all calls.
 */
/* P2 plateau: workbench mixed(constant:7, structural:48, schedule:33, register:81), 125 positional words; first owned mismatch +0x8 relocation.
 * The flag lattice and decrement-lifetime/root-initialization levers reduced 302 words to 125; mips2 retains exact size and frame.
 * Ninety-four opaque relocation identities and 31 downstream initialization/register-web opcode differences remain. */
#ifdef NON_MATCHING
void func_overlay_101_F000C6E8_18E7F08(void) {
    s32 count;
    s32 orderIndex;
    s32 nodeIndex;
    s32 priorNodeIndex;
    u8 selector;
    void *handle;
    O101TailC6E8Node32 *node32;
    O101TailC6E8Node20 *node20;
    s32 previousType;
    void *previous;
    u8 *cursor;
    u8 *end;

    if (o101TailC6E8StateReloc() == 0x12) {
        o101TailC6E8SpecialReloc(0);
        goto common_tail;
    }

    selector = gO101TailC6E8Selector;
    count = gO101TailC6E8QueueCount;
    if (count > 0) {
        gO101TailC6E8QueueCount--;
        if (gO101TailC6E8QueueCount > 0) {
            cursor = gO101TailC6E8QueueBytes;
            end = cursor + gO101TailC6E8QueueCount;
            do {
                *cursor = cursor[1];
                cursor++;
            } while (cursor < end);
        }
    }

    switch (selector) {
        case 0:  o101TailC6E8Block00Reloc(); break;
        case 1:  o101TailC6E8Block01Reloc(); break;
        case 2:  o101TailC6E8Block02Reloc(); break;
        case 3:  o101TailC6E8Block03Reloc(); break;
        case 4:  o101TailC6E8Block04Reloc(); break;
        case 5:  o101TailC6E8Block05Reloc(); break;
        case 6:  o101TailC6E8Block06Reloc(); break;
        case 7:  o101TailC6E8Block07Reloc(); break;
        case 8:  o101TailC6E8Block08Reloc(); break;
        case 9:  o101TailC6E8Block09Reloc(); break;
        case 10: overlay101BuildPresentationA(); break;
        case 11: o101TailC6E8Block11Reloc(); break;
        case 12: o101TailC6E8Block12Reloc(); break;
        case 13: o101TailC6E8Block13Reloc(); break;
        case 14: o101TailC6E8Block14Reloc(0); break;
        case 15: o101TailC6E8Block15Reloc(1); break;
        case 16: o101TailC6E8Block16Reloc(2); break;
        case 17: o101TailC6E8Block17Reloc(3); break;
        case 18: o101TailC6E8Block18Reloc(4); break;
        case 19: o101TailC6E8Block19Reloc(5); break;
        case 20: o101TailC6E8Block20Reloc(); break;
        case 21: o101TailC6E8Block21Reloc(); break;
        case 22: o101TailC6E8Block22Reloc(0); break;
        case 23: o101TailC6E8Block23Reloc(1); break;
        case 24: o101TailC6E8Block24Reloc(); break;
        default: o101TailC6E8DefaultReloc(0); break;
    }

    if (gO101TailC6E8PresentationActive != 0) {
        orderIndex = gO101TailC6E8OrderCount;
        gO101TailC6E8Root.height = 0x120;
        gO101TailC6E8Root.width = 0x180;
        gO101TailC6E8Root.y = -0x18;
        gO101TailC6E8Root.x = -0x20;
        gO101TailC6E8Root.kind = 2;
        gO101TailC6E8Root.asset = &gO101TailC6E8AssetDF0;
        gO101TailC6E8Root.color0 = 0xFF;
        gO101TailC6E8Root.color1 = 0xFF;
        gO101TailC6E8Root.value0A = 0;
        gO101TailC6E8Root.value0C = 0;
        gO101TailC6E8Root.chainType = 0;
        gO101TailC6E8Root.chain = NULL;
        gO101TailC6E8OrderSlots[orderIndex] = &gO101TailC6E8Root;
        gO101TailC6E8OrderCount = orderIndex + 1;

        nodeIndex = gO101TailC6E8Node32Count;
        node32 = &gO101TailC6E8Nodes32[nodeIndex];
        node32->x = 0x35;
        node32->y = 0xE0;
        node32->scale = 1.0f;
        node32->value10 = 0;
        node32->color0 = 0xFF;
        node32->color1 = 0;
        node32->value14 = 0.0f;
        node32->value18 = 0;
        handle = o101TailC6E8Create8FReloc(
            0x8F, NULL, &gO101TailC6E8Root,
            &gO101TailC6E8OrderCount);
        nodeIndex = gO101TailC6E8Node32Count;
        node32 = &gO101TailC6E8Nodes32[nodeIndex];
        previousType = gO101TailC6E8Root.chainType;
        previous = gO101TailC6E8Root.chain;
        gO101TailC6E8Root.chainType = 2;
        gO101TailC6E8Root.chain = node32;
        node32->previousType = previousType;
        node32->previous = previous;
        node32->handle = handle;
        gO101TailC6E8Node32Count = nodeIndex + 1;

        priorNodeIndex = nodeIndex;
        nodeIndex = gO101TailC6E8Node32Count;
        node32 = &gO101TailC6E8Nodes32[nodeIndex];
        node32->x = 0xC0;
        node32->y = 0xE0;
        node32->scale = 1.0f;
        node32->value10 = 0;
        node32->color0 = 0xFF;
        node32->color1 = 0;
        node32->value14 = 0.0f;
        node32->value18 = 0;
        handle = o101TailC6E8Create90Reloc(
            0x90, NULL, &gO101TailC6E8Root, priorNodeIndex);
        nodeIndex = gO101TailC6E8Node32Count;
        node32 = &gO101TailC6E8Nodes32[nodeIndex];
        previousType = gO101TailC6E8Root.chainType;
        previous = gO101TailC6E8Root.chain;
        gO101TailC6E8Root.chainType = 2;
        gO101TailC6E8Root.chain = node32;
        node32->previousType = previousType;
        node32->previous = previous;
        node32->handle = handle;
        gO101TailC6E8Node32Count = nodeIndex + 1;

        priorNodeIndex = nodeIndex;
        nodeIndex = gO101TailC6E8Node20Count;
        node20 = &gO101TailC6E8Nodes20[nodeIndex];
        node20->x = 0x20;
        node20->y = 0x18;
        node20->scale = 1.0f;
        handle = o101TailC6E8CreateCompactReloc(
            1, node20, &gO101TailC6E8Root, priorNodeIndex);
        nodeIndex = gO101TailC6E8Node20Count;
        node20 = &gO101TailC6E8Nodes20[nodeIndex];
        previousType = gO101TailC6E8Root.chainType;
        previous = gO101TailC6E8Root.chain;
        gO101TailC6E8Root.chainType = 1;
        gO101TailC6E8Root.chain = node20;
        node20->previousType = previousType;
        node20->previous = previous;
        node20->handle = handle;
        gO101TailC6E8Node20Count = nodeIndex + 1;

        gO101TailC6E8Handle1D4 = o101TailC6E8Acquire5F1Reloc(0x5F1);
        gO101TailC6E8Handle1F4 = o101TailC6E8Acquire5F2Reloc(0x5F2);
    }

    gO101TailC6E8Handle33C = o101TailC6E8Acquire58Reloc(0x58);
    gO101TailC6E8Handle338 =
        o101TailC6E8Acquire338Reloc(0, 0x278D00);

common_tail:
    gO101TailC6E8PresentationDone = 0;
    o101TailC6E8Tail27Reloc(0x27);
    o101TailC6E8TailFinalReloc();
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o101/overlay101TailC6E8/func_overlay_101_F000C6E8_18E7F08.s")
#endif
