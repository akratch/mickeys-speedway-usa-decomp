#include "PR/ultratypes.h"

typedef struct Overlay44FrameSource {
    s16 dimension0;
    s16 dimension1;
    s16 frameCount;
    u8 storageMode;
    u8 speed;
    u8 *data;
    s32 frameSize;
} Overlay44FrameSource;

typedef struct Overlay44AnimationState {
    s8 sourceIndex;
    u8 mode;
    u8 flags;
    u8 reserved3;
    s32 phase;
    u8 reserved8[6];
    s8 protectedSlot0;
    s8 protectedSlot1;
    s8 cachedFrame[4];
    void *handles[4];
} Overlay44AnimationState;

extern Overlay44FrameSource *gOverlay44FrameSources;
extern void overlay44UploadFrameReloc(s32 operation, void *handle,
                                      void *source, s32 size);

/* Semantic clean-room draft for overlay 44 +0x294. */
void overlay44UpdateFrameCache(Overlay44AnimationState *state,
                               s32 updateRate) {
    volatile Overlay44FrameSource *source;
    s32 limit;
    s32 delta;
    s32 frame;
    s32 nextFrame;
    s32 frameSlot;
    s32 nextSlot;
    s32 slot;

    if ((state == 0) || (state->sourceIndex == -1)) {
        return;
    }

    source = &gOverlay44FrameSources[state->sourceIndex];
    if (state->flags & 4) {
        limit = (source->frameCount << 8) - 1;
    } else {
        limit = (source->frameCount - 1) << 8;
    }

    if (state->flags & 0x80) {
        state->flags &= ~0x80;
    } else {
        delta = ((source->speed << 8) / 60) * updateRate;
        do {
            if (state->flags & 1) {
                state->phase -= delta;
                if (state->phase < 0) {
                    if (state->flags & 4) {
                        if (state->flags & 2) {
                            state->phase = -state->phase;
                            state->flags &= ~1;
                        } else {
                            state->phase += limit;
                        }
                    } else {
                        state->phase = 0;
                    }
                }
            } else {
                state->phase += delta;
                if (state->phase > limit) {
                    if (state->flags & 2) {
                        state->phase = limit - (state->phase - limit);
                        state->flags |= 1;
                    } else if (state->flags & 4) {
                        state->phase -= limit;
                    } else {
                        state->phase = limit;
                    }
                }
            }
        } while ((state->phase < 0) || (state->phase > limit));
    }

    frame = state->phase >> 8;
    nextFrame = frame + 1;
    if (nextFrame >= source->frameCount) {
        nextFrame = 0;
    }

    frameSlot = -1;
    nextSlot = -1;
    slot = 3;
    do {
        if (state->cachedFrame[slot] == frame) frameSlot = slot;
        if (state->cachedFrame[slot] == nextFrame) nextSlot = slot;
    } while (slot--);

    if (frameSlot < 0) {
        slot = 3;
        do {
            if ((slot != nextSlot) && (slot != state->protectedSlot0) &&
                (slot != state->protectedSlot1)) {
                state->cachedFrame[slot] = frame;
                overlay44UploadFrameReloc(
                    0x42, state->handles[slot],
                    source->data + (frame * source->frameSize),
                    source->frameSize);
                frameSlot = slot;
                break;
            }
        } while (slot--);
    }

    if (nextSlot < 0) {
        slot = 3;
        do {
            if ((slot != frameSlot) && (slot != state->protectedSlot0) &&
                (slot != state->protectedSlot1)) {
                state->cachedFrame[slot] = nextFrame;
                overlay44UploadFrameReloc(
                    0x42, state->handles[slot],
                    source->data + (nextFrame * source->frameSize),
                    source->frameSize);
                nextSlot = slot;
                break;
            }
        } while (slot--);
    }

    state->protectedSlot0 = frameSlot;
    state->protectedSlot1 = nextSlot;
}
