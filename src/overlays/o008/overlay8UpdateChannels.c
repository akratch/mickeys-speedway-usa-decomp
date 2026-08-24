#include "PR/ultratypes.h"

typedef struct Overlay8ChannelState {
    u8 pad000[4];
    f32 position;
    u8 pad008[0x114];
    f32 values[4];
    u8 modes[4];
    u8 phases[4];
    u8 pad134[0x188];
    s32 selectorMode;
    u8 pad2C0[0x60];
    u8 selectors[4];
} Overlay8ChannelState;

extern const f32 gOverlay8PhaseScales[];
extern const f32 gOverlay8SelectorScales[];

extern f32 overlay8SampleChannel(f32 argument, void *sampleState);
extern void overlay8EmitChannel(Overlay8ChannelState *state, s32 kind,
                                f32 scale);

void overlay8UpdateChannels(void *unused, Overlay8ChannelState *state,
                            f32 gate, void *sampleState) {
    f32 factor;
    f32 maximumFactor;
    f32 sample;
    f32 position;
    register const f32 upper = 0.1f;
    register const f32 lower = -0.1f;
    register const f32 multiplier = -0.67f;
    s32 i;
    s32 selectorIndex;
    const f32 *selectorScales;

    selectorScales = gOverlay8SelectorScales;
    maximumFactor = 0.0f;
    for (i = 0; i < 4; i++) {
        if (gate < 8.0f) {
            if (state->modes[i] != 0) {
                state->modes[i] = 2;
                state->values[i] = state->values[i] * multiplier;
                if ((lower < state->values[i]) &&
                    (state->values[i] < upper)) {
                    state->modes[i] = 0;
                }
            } else {
                if (state->selectorMode == 1) {
                    selectorIndex = 0;
                } else {
                    selectorIndex = i;
                }
                factor = selectorScales[
                    state->selectors[selectorIndex] & 0xF];
                *(volatile f32 *) &state->values[i] =
                    gOverlay8PhaseScales[state->phases[i]] * state->position;
                state->values[i] *= factor;
                if (maximumFactor < factor) {
                    maximumFactor = factor;
                }
            }
        } else {
            sample = overlay8SampleChannel(0.95f, sampleState);
            state->values[i] +=
                (-4.0f - state->values[i]) * (1.0f - sample);
            state->modes[i] = 1;
        }

        state->phases[i] = (state->phases[i] + 1) & 0x1F;
    }

    if (0.05f <= maximumFactor) {
        position = state->position;
        if ((position < -4.5f) || (position > 4.5f)) {
            overlay8EmitChannel(state, 0x28, 0.15f);
        }
    }
}
