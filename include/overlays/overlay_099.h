#ifndef OVERLAY_099_H
#define OVERLAY_099_H

#include "ultra64.h"
#include "n_audio/gbi.h"

typedef struct Overlay99Vec3 {
    f32 x;
    f32 y;
    f32 z;
} Overlay99Vec3;

typedef struct Overlay99RenderEntry {
    u8 *spriteData;
    s8 tableIndex;
    u8 pad05[3];
    f32 scale;
    u8 pad0C[8];
} Overlay99RenderEntry;

typedef struct Overlay99TableOwner {
    u8 pad00[0x40];
    Overlay99Vec3 *vectors;
} Overlay99TableOwner;

typedef struct Overlay99RenderState {
    s16 rotation0;
    s16 rotation1;
    s16 heading;
    u16 flags;
    f32 transformScale;
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x21];
    u8 mode39;
    u8 pad3A[6];
    f32 *unitScale;
    u8 pad44[0x0C];
    f32 *opacity;
    u8 pad54[0x0C];
    Overlay99RenderEntry *entries;
    u8 pad64[4];
    Overlay99TableOwner **tableOwner;
    u8 pad6C[0x20];
    u8 entryCount;
} Overlay99RenderState;

typedef struct Overlay99CameraSprite {
    s16 angle;
    s16 frame;
    u16 pad04;
    u16 divisor;
    f32 transformScale;
    f32 matrixScale;
    f32 x;
    f32 y;
    f32 z;
    s32 frameCount;
    u8 *spriteData;
} Overlay99CameraSprite;

void overlay99RenderSortedEntries(Gfx **displayList, Mtx **matrices,
                                  void *vertices,
                                  Overlay99RenderState *state,
                                  f32 intensityScale);

#endif
