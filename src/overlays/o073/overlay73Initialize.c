#include "PR/ultratypes.h"

typedef struct Overlay73Vertex {
    s16 x;
    s16 y;
    s16 z;
    u8 r;
    u8 g;
    u8 b;
    u8 a;
} Overlay73Vertex;

typedef struct Overlay73State {
    Overlay73Vertex vertices[12];
    void *resource;
    u8 vertexBank;
    u8 active;
    s16 phase;
    f32 x;
    f32 y;
    f32 z;
    f32 scale;
    f32 timer;
    s16 phaseLimit;
    s16 status;
    s32 counter;
} Overlay73State;

typedef struct Overlay73Config {
    u8 pad00[0x22];
    s8 resourceCount;
} Overlay73Config;

typedef struct Overlay73Object {
    u8 pad00[0x08];
    f32 outputScale;
    u8 pad0C[0x14];
    f32 value;
    u8 pad24[0x1C];
    Overlay73Config *config;
    u8 pad44[0x08];
    f32 *output;
    u8 pad50[0x14];
    Overlay73State *state;
    void **resources;
} Overlay73Object;

typedef struct Overlay73Header {
    u8 pad00[4];
    s16 x;
    s16 y;
    s16 z;
    u16 scale;
    u8 resourceIndex;
    u8 outputScale;
} Overlay73Header;

extern s16 D_80[12][3];
extern f32 D_0;
extern f32 D_4;
extern f32 D_8;

/* DKR v77/v80 and JFG contain no exact donor for this initializer. */
/* Workbench: allocation-mismatch; 8/100 instruction words plus overlay D_0/D_4/D_8 relocation layout, frame exact.
 * Levers: loop/index lifetime and forms, full flag lattice, global-layout probes, and constant audit.
 * Remains: target a3 versus candidate a2 pool coloring and the local overlay constant relocation surface. */
#ifdef NON_MATCHING
void func_overlay_073_F0000000_18CAAC0(Overlay73Object *object,
                                       Overlay73Header *header,
                                       s32 preserveState) {
    s32 i;
    Overlay73State *state;
    Overlay73Vertex *vertex;
    s16 (*source)[3];

    state = object->state;
    if (preserveState == 0) {
        object->value = 0.0f;
        state->phase = 0;
        state->counter = 0;
        state->status = 0;
        state->active = 0;
        state->phaseLimit = 0x180;
        state->timer = 0.0f;
        vertex = state->vertices;
        source = D_80;
        i = 11;
        do {
            vertex->x = (*source)[0];
            vertex->y = (*source)[1];
            vertex->z = (*source)[2];
            vertex->r = 0xFF;
            vertex->g = 0xFF;
            vertex->b = 0xFF;
            vertex->a = 0xFF;
            vertex++;
            source++;
        } while (i--);
        state->vertexBank = 1;
    }

    i = header->resourceIndex;
    if (i >= object->config->resourceCount) {
        i = 0;
    }
    state->resource = object->resources[i];
    state->x = header->x;
    state->y = header->y;
    state->z = header->z;
    state->scale = header->scale;
    object->outputScale = (f32)header->outputScale * D_0;
    if (object->output != NULL) {
        object->output[0] = object->outputScale * D_4;
        object->output[1] = object->outputScale * D_8;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o073/overlay73Initialize/func_overlay_073_F0000000_18CAAC0.s")
#endif
