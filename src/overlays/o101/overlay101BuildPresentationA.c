#include "PR/ultratypes.h"

typedef struct Overlay101BuilderRoot {
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
    void *text50;
} Overlay101BuilderRoot;

typedef struct Overlay101Node32 {
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
} Overlay101Node32;

typedef struct Overlay101Node20 {
    s32 previousType;
    void *previous;
    s16 x;
    s16 y;
    f32 scale;
    void *handle;
} Overlay101Node20;

typedef struct Overlay101Node24 {
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
} Overlay101Node24;

/* Every A/B alias below is a distinct relocation identity, even where its
 * workbench link placeholder has the same encoded addend as its sibling. */
extern Overlay101BuilderRoot gOverlay101BuilderRoot;
extern s32 gOverlay101BuilderOrderCountA;
extern s32 gOverlay101BuilderOrderCountB;
extern void *gOverlay101BuilderOrderA[];
extern void *gOverlay101BuilderOrderB[];

extern s32 gOverlay101BuilderNode20CountA;
extern s32 gOverlay101BuilderNode20CountB;
extern Overlay101Node20 gOverlay101BuilderNodes20A[];
extern Overlay101Node20 gOverlay101BuilderNodes20B[];

extern s32 gOverlay101BuilderNode32CountA;
extern s32 gOverlay101BuilderNode32CountB;
extern Overlay101Node32 gOverlay101BuilderNodes32A[];
extern Overlay101Node32 gOverlay101BuilderNodes32B[];

extern s32 gOverlay101BuilderNode24CountA;
extern s32 gOverlay101BuilderNode24CountB;
extern Overlay101Node24 gOverlay101BuilderNodes24A[];
extern Overlay101Node24 gOverlay101BuilderNodes24B[];

extern u8 gOverlay101BuilderAssetA;
extern void *gOverlay101BuilderText;
extern u8 *gOverlay101BuilderInput12C;
extern u8 *gOverlay101BuilderInput130;

/* The two create calls intentionally have different arities. The old-style
 * declaration preserves that real call surface without inventing an ABI. */
extern void *overlay101BuilderCreateReloc();
extern void overlay101BuilderCreateFinalReloc(void *text, s32 index,
                                               s32 color, s32 *count);
extern s32 overlay101ByteLength(u8 *text);

/* Workbench: structure-mismatch; 167 raw/166 aligned words differ, with 208
 * target versus 209 candidate instructions and first structural divergence at
 * +0x10. The exact frame/CFG body still has one late dim-color materialization. */
#ifdef NON_MATCHING
void overlay101BuildPresentationA(void) {
    s32 orderIndex;
    s32 node32IndexA;
    s32 node32IndexB;
    s32 node20Index;
    s32 node24IndexA;
    s32 node24IndexB;
    s32 previousType;
    void *previous;
    void *handle;
    s8 length;
    f32 opacityScale;
    s32 dimColor;
    Overlay101Node32 *node32A;
    Overlay101Node32 *node32B;
    Overlay101Node20 *node20A;
    Overlay101Node20 *node20B;
    Overlay101Node24 *node24A;
    Overlay101Node24 *node24B;

    opacityScale = 1.0f;
    orderIndex = gOverlay101BuilderOrderCountA;
    gOverlay101BuilderRoot.height30 = 0xF0;
    gOverlay101BuilderRoot.width2E = 0x140;
    gOverlay101BuilderRoot.kind = 4;
    gOverlay101BuilderRoot.asset34 = &gOverlay101BuilderAssetA;
    gOverlay101BuilderRoot.color32 = 0xFF;
    gOverlay101BuilderRoot.color33 = 0xFF;
    gOverlay101BuilderRoot.value26 = 0;
    gOverlay101BuilderRoot.value28 = 0;
    gOverlay101BuilderRoot.value2A = 0;
    gOverlay101BuilderRoot.value2C = 0;
    gOverlay101BuilderRoot.chainType = 0;
    gOverlay101BuilderRoot.chain = NULL;
    gOverlay101BuilderOrderA[orderIndex] =
        &gOverlay101BuilderRoot.chainType;
    gOverlay101BuilderOrderCountA = orderIndex + 1;

    node32IndexA = gOverlay101BuilderNode32CountA;
    node32A = &gOverlay101BuilderNodes32A[node32IndexA];
    node32A->x = 0xF2;
    node32A->y = 0x14E;
    node32A->value10 = 0;
    node32A->color12 = 0xFF;
    node32A->color13 = 0;
    node32A->value18 = 0;
    node32A->scale = 1.0f;
    node32A->value14 = 0.0f;
    handle = overlay101BuilderCreateReloc(0x91, NULL, orderIndex);

    node32IndexB = gOverlay101BuilderNode32CountB;
    node32B = &gOverlay101BuilderNodes32B[node32IndexB];
    node32B->previousType = gOverlay101BuilderRoot.chainType;
    node32B->previous = gOverlay101BuilderRoot.chain;
    gOverlay101BuilderNode32CountB = node32IndexB + 1;
    gOverlay101BuilderRoot.chainType = 2;
    gOverlay101BuilderRoot.chain = node32B;
    node32B->handle = handle;

    orderIndex = gOverlay101BuilderOrderCountB;
    gOverlay101BuilderRoot.x42 = 0x20;
    gOverlay101BuilderRoot.y46 = 0x50;
    gOverlay101BuilderRoot.value4A = 0xA0;
    gOverlay101BuilderRoot.value4C = 0xAE;
    gOverlay101BuilderRoot.width44 = 0x18;
    gOverlay101BuilderRoot.height48 = 0x18;
    gOverlay101BuilderRoot.mode40 = 0;
    gOverlay101BuilderRoot.color4E = 0xFF;
    gOverlay101BuilderRoot.color4F = 0xFF;
    gOverlay101BuilderRoot.child = NULL;
    gOverlay101BuilderRoot.childType = 0;
    gOverlay101BuilderRoot.text50 = gOverlay101BuilderInput12C;
    gOverlay101BuilderOrderB[orderIndex] =
        &gOverlay101BuilderRoot.childType;
    gOverlay101BuilderOrderCountB = orderIndex + 1;

    node20Index = gOverlay101BuilderNode20CountA;
    node20A = &gOverlay101BuilderNodes20A[node20Index];
    node20A->x = 0x10;
    node20A->y = 0x16;
    node20A->scale = 1.0f;
    handle = overlay101BuilderCreateReloc(0x15, node20A, orderIndex,
                                          node32IndexB);

    node20Index = gOverlay101BuilderNode20CountB;
    node20B = &gOverlay101BuilderNodes20B[node20Index];
    previousType = gOverlay101BuilderRoot.childType;
    previous = gOverlay101BuilderRoot.child;
    gOverlay101BuilderNode20CountB = node20Index + 1;
    gOverlay101BuilderRoot.childType = 1;
    gOverlay101BuilderRoot.child = node20B;
    node20B->handle = handle;
    node20B->previousType = previousType;
    node20B->previous = previous;

    node24IndexA = gOverlay101BuilderNode24CountA;
    node24A = &gOverlay101BuilderNodes24A[node24IndexA];
    node24A->x = 0x50;
    node24A->y = 0x9C;
    length = overlay101ByteLength(gOverlay101BuilderInput130);

    dimColor = 0xC0;
    node24IndexB = gOverlay101BuilderNode24CountB;
    node24B = &gOverlay101BuilderNodes24B[node24IndexB];
    node24B->length = length;
    node24B->opacity =
        (s8)(s32)((f32)(u32)(length & 0xFF) * opacityScale);
    node24B->kind = 4;
    node24B->mode = 2;
    node24B->color0 = 0xFF;
    node24B->color1 = dimColor;
    node24B->color2 = dimColor;
    node24B->color3 = 0xFF;

    previousType = *(volatile s32 *)&gOverlay101BuilderRoot.childType;
    previous = *(void *volatile *)&gOverlay101BuilderRoot.child;
    *(volatile s32 *)&gOverlay101BuilderNode24CountB = node24IndexB + 1;
    gOverlay101BuilderRoot.child = node24B;
    gOverlay101BuilderRoot.childType = 3;
    node24B->previousType = previousType;
    node24B->previous = previous;
    node24B->text = gOverlay101BuilderInput130;

    overlay101BuilderCreateFinalReloc(&gOverlay101BuilderText, node24IndexB,
                                      dimColor,
                                      &gOverlay101BuilderNode24CountB);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o101/overlay101BuildPresentationA/func_overlay_101_F00099C4_18E51E4.s")
#endif

/* PLATEAU-HANDOFF:overlay101BuildPresentationA:start
 * symbol: overlay101BuildPresentationA
 * score: 166 differing words
 * frame: 0x20
 * relocations: 52
 * first-mismatch: +0x10
 * summary: 209/208 words and 167 raw differences; 28/34 target relocation offsets and types align, no identities resolve; next lever is the early order-count allocator web
 * PLATEAU-HANDOFF:overlay101BuildPresentationA:end
 */
