#include "PR/ultratypes.h"

#define O24_SHIFTL(value, shift, width) \
    ((u32)(((u32)(value) & ((1U << (width)) - 1U)) << (shift)))

typedef struct Overlay24Command {
    u32 w0;
    u32 w1;
} Overlay24Command;

typedef struct Overlay24Source {
    u8 pad00[0xC];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x38];
    f32 *opacity;
} Overlay24Source;

typedef struct Overlay24RenderState {
    u8 pad00[4];
    f32 yOffset;
    u8 pad08[4];
    s32 enabled;
    Overlay24Source *source;
} Overlay24RenderState;

typedef struct Overlay24RenderObject {
    u8 pad00[0xC];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x21];
    u8 opacity;
    u8 pad3A[0x2A];
    Overlay24RenderState *state;
    void **resource;
} Overlay24RenderObject;

extern s32 gOverlay24FadeActiveReloc;
extern s32 gOverlay24FadeScaleReloc;
extern void overlay24RenderHelperReloc(
    Overlay24Command **commands, void *arg1, void *arg2,
    Overlay24RenderObject *object, void *resource, s32 mode, s32 intensity);

void overlay24RenderState(Overlay24Command **commands, void *arg1, void *arg2,
                          Overlay24RenderObject *object) {
    s32 opacity;
    s32 alpha;
    Overlay24RenderState *state;
    Overlay24Source *source;
    Overlay24Command **renderCommands;

    state = (opacity = 0xFF, object->state);
    if (state->enabled == 0) {
        return;
    }

    source = state->source;
    renderCommands = commands;
    if (source == NULL) {
        return;
    }

    object->x = source->x;
    object->y = source->y + state->yOffset;
    object->z = source->z;

    if (source->opacity != NULL) {
        opacity = *source->opacity * 255.0f;
    } else {
        opacity = 0xFF;
    }
    if (gOverlay24FadeActiveReloc != 0) {
        opacity = (opacity * gOverlay24FadeScaleReloc) >> 8;
    }

    alpha = (object->opacity * state->enabled) >> 8;

    {
        Overlay24Command *command = (*commands)++;
        command->w1 = 0;
        command->w0 = O24_SHIFTL(0xE7, 24, 8);
    }

    {
        Overlay24Command *command = (*commands)++;
        command->w1 = (command->w0 = O24_SHIFTL(0xFA, 24, 8),
                       ((opacity & 0xFF) << 24) |
                       ((opacity & 0xFF) << 16) |
                       ((opacity & 0xFF) << 8) |
                       (alpha & 0xFF));
    }

    {
        Overlay24Command *command = (*commands)++;
        command->w1 = (command->w0 = O24_SHIFTL(0xFB, 24, 8),
                       O24_SHIFTL(0xFF, 24, 8) |
                       O24_SHIFTL(0xFF, 16, 8) |
                       O24_SHIFTL(0xFF, 8, 8) |
                       O24_SHIFTL(0, 0, 8));
    }

    overlay24RenderHelperReloc(renderCommands, arg1, arg2, object,
                               *object->resource, 0xC, 0xFF);

    {
        Overlay24Command *command = (*commands)++;
        command->w1 = (command->w0 = O24_SHIFTL(0xFA, 24, 8),
                       O24_SHIFTL(0xFF, 24, 8) |
                       O24_SHIFTL(0xFF, 16, 8) |
                       O24_SHIFTL(0xFF, 8, 8) |
                       O24_SHIFTL(0xFF, 0, 8));
    }
}
