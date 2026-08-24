#include "overlays/o085/state.h"

/*
 * Overlay 85 +0x000. The Epoch 5 DKR v77/v80 donor search was negative;
 * this is a local reconstruction from Mickey's own data flow.
 */
void overlay85Configure(Overlay85State *state, Overlay85Config *config) {
    f32 scale;
    scale = ((s32)config->scale) & 0xFF;
    if (scale < 10.0f) {
        scale = 10.0f;
    }
    scale *= 0.015625f;
    state->scale = state->resource->scale * scale;
    if (state->outputScale != NULL) {
        state->outputScale[0] = state->resource->outputScaleX * scale;
        state->outputScale[1] = state->resource->outputScaleY * scale;
    }
    state->frame = config->frame;
    state->angle = (((s32)config->angle) & 0xFF) << 10;
    if (state->frame >= state->resource->frameCount) {
        state->frame = 0;
    }
    *(s32 *)&state->timer = 0;
    state->unk88 = 0;
    if (state->output != NULL) {
        state->output->state = 2;
    }
}
