#include "PR/ultratypes.h"

/* Overlay 99 +0x800: sorted translucent-entry render pass. */

typedef struct Overlay99Gfx {
    u32 w0;
    u32 w1;
} Overlay99Gfx;

typedef struct Overlay99Vec3 {
    f32 x;
    f32 y;
    f32 z;
} Overlay99Vec3;

typedef struct Overlay99RenderEntry {
    s32 id;
    s8 tableIndex;
    u8 pad05[3];
    f32 scale;
    u8 pad0C[8];
} Overlay99RenderEntry;

typedef struct Overlay99TableOwner {
    u8 pad00[0x40];
    Overlay99Vec3 *vectors;
} Overlay99TableOwner;

typedef struct Overlay99Transform {
    u8 pad00[0x0C];
    f32 x;
    f32 y;
    f32 z;
} Overlay99Transform;

typedef struct Overlay99RenderState {
    u8 pad00[0x39];
    u8 mode39;
    u8 pad3A[6];
    f32 *unitScale;
    u8 pad44[0x0C];
    void *resource50;
    u8 pad54[0x0C];
    Overlay99RenderEntry *entries;
    u8 pad64[4];
    Overlay99TableOwner **tableOwner;
    u8 pad6C[0x20];
    u8 entryCount;
} Overlay99RenderState;

typedef struct Overlay99DrawRecord {
    s16 zeroA8;
    s16 intensity;
    s16 padAC;
    s16 three;
    f32 scaled;
    f32 one;
    u32 outB8;
    s16 outBC;
    s16 padBE;
    u32 outC0;
    s32 color;
    s32 id;
    void *matrix;
} Overlay99DrawRecord;

extern f32 overlay99Measure(f32 x, f32 y, f32 z);
extern Overlay99Transform *overlay99GetTransform(void);
extern void overlay99UpdateState(Overlay99RenderState *state);
extern void overlay99BuildMatrix(Overlay99RenderState *state, void **matrix);
extern void overlay99BuildRecord(void **matrix, f32 x, f32 y, f32 z,
                                 u32 *outB8, s16 *outBC, u32 *outC0);
extern void overlay99DrawEntry(Overlay99Gfx **displayList, void *arg1,
                               void *arg2, Overlay99RenderState *state,
                               void *resource, Overlay99DrawRecord *record,
                               s32 mode, s32 selector);
extern f32 D_8;
extern f32 D_4;

void overlay99RenderSortedEntries(Overlay99Gfx **displayList, void *arg1,
                                  void *arg2, Overlay99RenderState *state,
                                  f32 intensityScale) {
    volatile u8 framePad[0x0C];
    f32 savedX;
    f32 savedY;
    f32 savedZ;
    volatile u8 gapSavedDistances[0x14];
    f32 distances[4];
    volatile u8 gapDistancesRecord[0x3C];
    Overlay99DrawRecord record;
    volatile u8 gapRecordSorted[8];
    Overlay99RenderEntry *sorted[4];
    f32 invScale;
    Overlay99RenderEntry *entry;
    Overlay99TableOwner *owner;
    Overlay99Transform *transform;
    Overlay99Vec3 *vec;
    Overlay99Gfx *command;
    s32 count;
    s32 i;
    s32 j;

    entry = state->entries;
    owner = *state->tableOwner;
    count = 0;
    if (entry != 0) {
        i = 0;
        while ((i < state->entryCount) && (i != 4)) {
            vec = &owner->vectors[entry->tableIndex];
            distances[count] = overlay99Measure(vec->x, vec->y, vec->z);
            sorted[count] = entry;
            entry++;
            i++;
            count++;
        }
    }

    if (count <= 0) {
        return;
    }

    for (i = count - 1; i > 0; i--) {
        for (j = 0; j < i; j++) {
            if (distances[j + 1] < distances[j]) {
                invScale = distances[j];
                distances[j] = distances[j + 1];
                distances[j + 1] = invScale;
                entry = sorted[j];
                sorted[j] = sorted[j + 1];
                sorted[j + 1] = entry;
            }
        }
    }

    transform = overlay99GetTransform();
    savedX = transform->x;
    savedY = transform->y;
    savedZ = transform->z;
    transform->x = 0.0f;
    transform->y = 0.0f;
    transform->z = D_4;
    overlay99UpdateState(state);
    transform->x = savedX;
    transform->y = savedY;
    transform->z = savedZ;

    invScale = 1.0f / *state->unitScale;
    overlay99BuildMatrix(state, &record.matrix);
    record.intensity = (s16)(s32)(intensityScale * D_8);
    record.zeroA8 = 0;
    record.three = 3;
    record.color = 0x3333;
    record.one = 1.0f;

    command = *displayList;
    *displayList = command + 1;
    command->w1 = 0;
    command->w0 = 0xE7000000;
    command = *displayList;
    *displayList = command + 1;
    command->w1 = 0xFFFFFF00;
    command->w0 = 0xFB000000;

    for (i = 0; i < count; i++) {
        entry = sorted[i];
        vec = &owner->vectors[entry->tableIndex];
        record.scaled = entry->scale * invScale;
        record.id = entry->id;
        overlay99BuildRecord(&record.matrix, vec->x, vec->y, vec->z,
                             &record.outB8, &record.outBC, &record.outC0);
        overlay99DrawEntry(displayList, arg1, arg2, state, state->resource50,
                           &record, 14, state->mode39);
    }
}
