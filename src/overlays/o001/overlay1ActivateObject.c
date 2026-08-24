#include "PR/ultratypes.h"

typedef struct Overlay1OwnerState {
    u8 pad000[0x37C];
    s16 recordIndex;
    u8 selector;
} Overlay1OwnerState;

typedef struct Overlay1Owner {
    u8 pad000[0x64];
    Overlay1OwnerState *state;
} Overlay1Owner;

typedef struct Overlay1Sample {
    f32 x;
    f32 y;
    u8 pad008[4];
    s8 enabled;
} Overlay1Sample;

extern Overlay1Owner *D_1D9C;
extern Overlay1OwnerState *D_1DA0;
extern Overlay1OwnerState *D_1DA0Read;
extern void *D_1D58;
extern Overlay1Sample *D_1D60;
extern Overlay1Sample *D_1D64;
extern Overlay1Sample *volatile D_1D68;
extern Overlay1Sample *D_1D68Read;
extern Overlay1Sample *D_1D6C;
extern Overlay1Sample *D_0208;
extern Overlay1Sample *D_020C;
extern Overlay1Sample *D_0210;
extern Overlay1Sample *D_0214;
extern s32 D_0;
extern f32 D_B0;

extern void *overlay1Chain0ContextReloc(void *source, void *context);
extern void *overlay1Chain0Reloc(void *source);
extern void *overlay1Chain40Reloc(void *source);
extern f32 overlay1InterpolateReloc(f32 first, f32 second, s32 third,
                                   s32 fourth, f32 weight);

s32 overlay1ActivateObject(Overlay1Owner *owner) {
    Overlay1OwnerState *state;
    Overlay1Sample *record;
    register Overlay1Sample *volatile *recordSlot;
    s32 index;

    D_1D9C = 0;
    D_1DA0 = 0;
    if (owner == 0) {
        return 0;
    }
    D_1D9C = owner;
    state = owner->state;
    if (state == 0) {
        return 0;
    }
    D_1DA0 = state;
    if (D_0 == 1) {
        state = *(Overlay1OwnerState *volatile *)&D_1DA0;
        index = state->recordIndex;
        record = (Overlay1Sample *)((u8 *)D_1D58 + index * 0x94);
        recordSlot = &D_1D68;
        *recordSlot = record;
        D_1D64 = overlay1Chain0ContextReloc(record, &D_1D9C);
        D_1D60 = overlay1Chain0Reloc(D_1D64);
        D_1D6C = overlay1Chain40Reloc(D_1D68Read);
        state = D_1DA0Read;
        D_0208 =
            (Overlay1Sample *)((u8 *)D_1D60 + state->selector * 0x10 + 0x14);
        D_020C =
            (Overlay1Sample *)((u8 *)D_1D64 + state->selector * 0x10 + 0x14);
        D_0210 =
            (Overlay1Sample *)((u8 *)D_1D68Read + state->selector * 0x10 + 0x14);
        D_0214 =
            (Overlay1Sample *)((u8 *)D_1D6C + state->selector * 0x10 + 0x14);
    }
    return 1;
}

s32 overlay1FindClosestSample(f32 x, f32 y, Overlay1Sample *source,
                              f32 weight) {
    Overlay1Sample *first;
    Overlay1Sample *current;
    Overlay1Sample *third;
    Overlay1Sample *fourth;
    f32 bestDistance;
    s32 bestIndex;
    s32 index;

    bestDistance = D_B0;
    bestIndex = -1;
    first = (Overlay1Sample *)((u8 *)overlay1Chain0Reloc(source) + 0x84);
    current = (Overlay1Sample *)((u8 *)source + 0x84);
    third = (Overlay1Sample *)((u8 *)overlay1Chain40Reloc(source) + 0x84);
    fourth = (Overlay1Sample *)((u8 *)overlay1Chain40Reloc(
                                   (u8 *)third - 0x84) + 0x84);
    index = 7;
    do {
        if (current->enabled != 0) {
            f32 sampleX;
            f32 sampleY;
            f32 dx;
            f32 dy;
            f32 distance;

            sampleX = overlay1InterpolateReloc(first->x, current->x,
                                               *(s32 *)&third->x,
                                               *(s32 *)&fourth->x, weight);
            sampleY = overlay1InterpolateReloc(first->y, current->y,
                                               *(s32 *)&third->y,
                                               *(s32 *)&fourth->y, weight);
            dx = x - sampleX;
            dy = y - sampleY;
            distance = dx * dx + dy * dy;
            if (distance < bestDistance) {
                bestDistance = distance;
                bestIndex = index;
            }
        }
        first--;
        current--;
        third--;
        fourth--;
    } while (index--);
    return bestIndex;
}
