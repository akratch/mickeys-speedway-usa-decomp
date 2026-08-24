#include "PR/ultratypes.h"

typedef struct Gfx {
    u32 w0;
    u32 w1;
} Gfx;

typedef struct Overlay23RenderState {
    u8 pad00[8];
    f32 scale;
    u8 pad0C[4];
    s16 value0;
    s16 value1;
    u8 pad14[0x0C];
    s32 resource;
} Overlay23RenderState;

typedef struct Overlay23RenderObject {
    u8 pad00[0x0C];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x21];
    u8 alpha;
    u8 pad3A[0x16];
    void *renderResource;
    u8 pad54[0x10];
    Overlay23RenderState *state;
} Overlay23RenderObject;

typedef struct Overlay23RenderPacket {
    s16 value0;
    s16 value1;
    u8 pad04[2];
    s16 mode;
    f32 scale;
    f32 unitScale;
    f32 x;
    f32 y;
    f32 z;
    s32 color;
    s32 resource;
} Overlay23RenderPacket;

typedef struct Overlay23AlignedRenderPacket {
    u8 pad00[8];
    Overlay23RenderPacket packet;
} Overlay23AlignedRenderPacket;

extern void overlay23PrepareRenderReloc(void);
extern void overlay23SubmitRenderReloc(
    Gfx **displayList, s32 renderArg0, s32 renderArg1,
    Overlay23RenderObject *object, void *renderResource,
    Overlay23RenderPacket *packet, s32 mode, s32 alpha);

/* The live wrapper preserves the shipped packet's stack placement. */
#ifdef NON_MATCHING
void overlay23RenderEffect(Overlay23RenderObject *object, Gfx **displayList,
                           s32 renderArg0, s32 renderArg1) {
    Overlay23RenderState *state;
    Overlay23AlignedRenderPacket packetStorage;
    Gfx *command;

    state = object->state;
    overlay23PrepareRenderReloc();

    packetStorage.packet.mode = 3;
    packetStorage.packet.color = 0x3333;
    packetStorage.packet.value0 = state->value0;
    packetStorage.packet.value1 = state->value1;
    packetStorage.packet.scale = state->scale;
    packetStorage.packet.unitScale = 1.0f;
    packetStorage.packet.x = object->x;
    packetStorage.packet.y = object->y;
    packetStorage.packet.z = object->z;
    packetStorage.packet.resource = state->resource;

    command = (*displayList)++;
    command->w1 = 0;
    command->w0 = 0xE7000000;
    command = (*displayList)++;
    command->w1 = 0xFFFFFF00;
    command->w0 = 0xFB000000;

    overlay23SubmitRenderReloc(displayList, renderArg0, renderArg1, object,
                               object->renderResource, &packetStorage.packet,
                               14, object->alpha);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o023/overlay23RenderEffect/func_overlay_023_F0000468_1879678.s")
#endif
