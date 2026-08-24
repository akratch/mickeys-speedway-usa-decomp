#include "PR/ultratypes.h"

typedef struct Overlay43Command {
    s16 type;
    u8 mode;
    u8 byte03;
    s16 short04;
    s16 width;
    s16 height;
    s16 short0A;
    s16 short0C;
    s16 scale;
    s16 short10;
    s16 short12;
    void *data;
    s16 short18;
    u8 byte1A;
    u8 byte1B;
    u8 byte1C;
    u8 byte1D;
    u8 byte1E;
    u8 byte1F;
} Overlay43Command;

typedef struct Overlay43State {
    f32 x;
    f32 y;
    Overlay43Command *command;
    s16 short0C;
    s16 short0E;
    u8 flags;
    u8 byte11;
    u8 byte12;
    u8 byte13;
    u8 pad14[0x0C];
    u8 *bufferEnd;
    u8 pad24[0x3C];
    void **current;
    void **parent;
    u8 pad68[0x44];
    f32 alpha;
    u8 padB0[0x0A];
    u8 enabled;
} Overlay43State;

typedef struct Overlay43Source {
    f32 x;
    f32 y;
    u8 pad08[8];
    u8 flags;
    u8 byte11;
    u8 byte12;
    u8 pad13[9];
    Overlay43State *state;
} Overlay43Source;

typedef struct Overlay43Parent {
    u8 pad00[0x60];
    s8 linkIndex;
} Overlay43Parent;

typedef struct Overlay43Input {
    u8 pad00[0x3A];
    s8 currentIndex;
    u8 pad3B[5];
    Overlay43Parent *parent;
    u8 pad44[8];
    Overlay43Source *source;
    u8 pad50[0x18];
    void ***entries;
} Overlay43Input;

extern u8 D_0[];
extern s32 func_overlay_043_F0001184_188B154(
    Overlay43Input *input, Overlay43State *state);
extern void func_overlay_043_F0001264_188B234(
    Overlay43Input *input, Overlay43State *state, s32 flags, s32 mode);

#ifdef NON_MATCHING
s32 func_overlay_043_F0000000_1889FD0(Overlay43Input *input) {
    Overlay43Command *command;
    Overlay43State *state;
    void **current;
    void **selected;
    s8 linkIndex;
    u8 byte12;

    state = input->source->state;
    if (func_overlay_043_F0001184_188B154(input, state) == 0) {
        return 0;
    }

    state->alpha = 1.0f;
    current = input->entries[input->currentIndex];
    state->current = current;
    linkIndex = input->parent->linkIndex;
    if (linkIndex >= 0) {
        selected = current;
        state->parent = input->entries[linkIndex];
    } else {
        selected = state->current;
        state->parent = selected;
    }
    if (((u8 *)*selected)[0x2F] != 0) {
        state->enabled = 1;
    }

    func_overlay_043_F0001264_188B234(input, state, 0x800, 0xB);
    command = (Overlay43Command *)(state->bufferEnd - 0x20);
    command->type = 1;
    command->mode = 5;
    command->byte03 = 0;
    command->short04 = 0;
    command->width = 0x40;
    command->height = 0x40;
    command->short0A = 0;
    command->short0C = 0;
    command->scale = 0x1000;
    command->short10 = 0;
    command->short12 = 0;
    command->data = D_0;
    command->short18 = 0x38;
    command->byte1A = 0;
    command->byte1B = 0;
    command->byte1C = 2;
    command->byte1D = 0;
    command->byte1E = 2;
    command->byte1F = 0;
    state->command = command;

    state->x = input->source->x;
    state->y = input->source->y;
    state->flags = input->source->flags | 0x10;
    state->byte11 = input->source->byte11;
    byte12 = input->source->byte12;
    state->byte13 = 0;
    state->short0C = 0;
    state->short0E = 0;
    state->byte12 = byte12;
    return 1;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o043/overlay43InitializeState/func_overlay_043_F0000000_1889FD0.s")
#endif
