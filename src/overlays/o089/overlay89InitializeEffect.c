#include "PR/ultratypes.h"

typedef struct Overlay89EffectState {
    u8 mode;
    u8 particleCount;
    u8 index;
    u8 count;
    u8 red;
    u8 green;
    u8 blue;
    u8 intensity;
    u8 enabled;
    u8 pad09;
    s16 timer;
    f32 squaredRange;
    s16 minX;
    s16 minY;
    s16 left;
    s16 right;
    s16 top;
    s16 bottom;
    s32 motionWord;
    void *primaryHandle;
    f32 radius;
    f32 scale;
    void *secondaryHandle;
} Overlay89EffectState;

typedef struct Overlay89ColorEntry {
    u8 pad00[6];
    u8 red;
    u8 green;
    u8 blue;
    u8 pad09;
} Overlay89ColorEntry;

typedef struct Overlay89NestedHeader {
    u8 pad00[0x12];
    s16 count;
    u8 pad14[8];
    Overlay89ColorEntry *entries;
} Overlay89NestedHeader;

typedef struct Overlay89NestedRoot {
    Overlay89NestedHeader *header;
    Overlay89ColorEntry *colors;
} Overlay89NestedRoot;

typedef struct Overlay89Status {
    u8 pad00[0x132];
    s16 initialized;
} Overlay89Status;

typedef struct Overlay89Object {
    s16 angleA;
    s16 angleB;
    u8 pad04[4];
    f32 size;
    u8 pad0C[0x2E];
    u8 active;
    u8 pad3B[0x1D];
    Overlay89Status *status;
    u8 pad5C[8];
    Overlay89EffectState *state;
    Overlay89NestedRoot **nested;
} Overlay89Object;

typedef struct Overlay89Init {
    u8 pad00[0xA];
    u8 angleA;
    u8 angleB;
    s8 minX;
    s8 minY;
    u8 halfWidth;
    u8 halfHeight;
    u8 mode;
    u8 index;
    u8 particleCount;
    u8 count;
    u8 red;
    u8 green;
    u8 blue;
    u8 intensity;
    u8 range;
    u8 flags;
    u8 descriptorMode;
    u8 pad1B;
    u16 squaredRange;
    u8 pad1E;
    u8 size;
    u8 radius;
    u8 scale;
} Overlay89Init;

typedef struct Overlay89CreateDescriptor {
    u8 mode;
    s8 kind;
    u8 flags;
    s8 sentinelByte;
    s16 zero04;
    s16 zero06;
    s16 zero08;
    s16 zero0A;
    s16 sizeA;
    s16 sizeB;
    u8 red;
    u8 green;
    u8 blue;
    u8 intensity;
    s16 sentinel;
    s8 zero16;
    s8 zero17;
} Overlay89CreateDescriptor;

extern f32 gOverlay89InitScale[];
extern u8 overlay89Evaluate(Overlay89EffectState *state);
extern void *overlay89CreatePrimaryReloc(Overlay89Object *object,
                                         Overlay89CreateDescriptor *descriptor,
                                         Overlay89EffectState *state,
                                         Overlay89Init *init);
extern void overlay89MaintainReloc(Overlay89Object *object,
                                   Overlay89EffectState *state);

/* DKR v77/v80 and JFG contain no exact donor for this initializer. */
/*
 * Plateau retry (2026-08-25): -O2 -mips2 is exact size with 58 masked word
 * differences, first at +0x40. Assigning primaryHandle through state and
 * matching descriptor-store order improved the prior 94-word result, but the
 * target still preserves state in a caller-save slot across create and
 * maintain while this candidate reloads it. Lifetime splits and a cached
 * pointer regressed. A 10-minute permuter batch improved score 670 to 225 only
 * by combining that cache with an empty self-conjunction block, rejected as
 * scheduling scaffolding.
 */
#ifdef NON_MATCHING
void overlay89InitializeEffect(Overlay89Object *object,
                               Overlay89Init *init) {
    Overlay89EffectState *state;
    Overlay89CreateDescriptor descriptor;
    Overlay89NestedRoot *root;
    Overlay89ColorEntry *source;
    Overlay89ColorEntry *colors;
    s32 count;
    f32 range;
    f32 size;

    object->angleA = init->angleA << 8;
    state = object->state;
    object->angleB = init->angleB << 8;

    size = (f32)(u32)init->size;
    object->size = size * gOverlay89InitScale[1];

    state->mode = init->mode;
    state->particleCount = init->particleCount;
    state->index = init->index;
    state->count = init->count;
    state->red = init->red;
    state->green = init->green;
    state->blue = init->blue;
    state->intensity = init->intensity;
    state->enabled = 1;
    state->timer = 0;

    range = (f32)(u32)init->squaredRange;
    state->squaredRange = range * range;
    state->minX = init->minX << 6;
    state->minY = init->minY << 6;
    state->left = object->angleA - (init->halfWidth << 7);
    state->right = object->angleA + (init->halfWidth << 7);
    state->top = object->angleB - (init->halfHeight << 7);
    state->bottom = object->angleB + (init->halfHeight << 7);
    state->motionWord = 0;

    state->enabled = overlay89Evaluate(state);
    state->primaryHandle = NULL;
    state->secondaryHandle = NULL;

    descriptor.mode = init->descriptorMode;
    descriptor.kind = 3;
    descriptor.flags = (~init->flags) & 0x61;
    descriptor.sentinelByte = -1;
    descriptor.zero04 = 0;
    descriptor.zero06 = 0;
    descriptor.zero08 = 0;
    descriptor.zero0A = 0;
    descriptor.sizeA = init->range * 8;
    descriptor.sizeB = init->range * 8;
    descriptor.red = init->red;
    descriptor.green = init->green;
    descriptor.blue = init->blue;
    descriptor.intensity = state->enabled ? init->intensity : 0;
    descriptor.sentinel = -1;
    descriptor.zero16 = 0;
    descriptor.zero17 = 0;
    if (init->descriptorMode == 3) {
        descriptor.flags |= 0x40;
    }

    state->primaryHandle =
        overlay89CreatePrimaryReloc(object, &descriptor, state, init);
    state = object->state;
    state->scale = (f32)(init->scale * 8);
    state->radius = (f32)(init->radius * 8);

    if (init->descriptorMode == 3 && !(init->flags & 0x40)) {
        overlay89MaintainReloc(object, object->state);
    }

    object->status->initialized = 1;
    object->active = 0;

    root = *object->nested;
    if (root != NULL) {
        state = object->state;
        source = root->header->entries;
        colors = root->colors;
        count = root->header->count;
        while (count--) {
            if (source->red == 0 && source->green == 0 && source->blue == 0) {
                colors->red = state->red;
                colors->green = state->green;
                colors->blue = state->blue;
            }
            source++;
            colors++;
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o089/overlay89InitializeEffect/func_overlay_089_F0000270_18D44A0.s")
#endif
