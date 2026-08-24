#include "PR/ultratypes.h"

typedef struct Overlay29Resource {
    u8 pad0[4];
    s16 first;
    u8 pad6[2];
    u8 last;
} Overlay29Resource;

typedef struct Overlay29Owner {
    u8 pad00[0x64];
    Overlay29Resource *resource;
} Overlay29Owner;

/* Fresh pinned DKR v77/v80 and JFG scans found no exact donor for this call chain. */
extern s32 overlay29CreateReloc(s32 value);
extern s32 gOverlay29Node0;
extern s32 gOverlay29Node4;
extern s32 gOverlay29Node8;
extern s32 gOverlay29NodeC;

void overlay29BuildChain(Overlay29Owner *owner) {
    Overlay29Resource *resource;

    resource = owner->resource;
    gOverlay29Node8 = overlay29CreateReloc(resource->first);
    gOverlay29Node4 = overlay29CreateReloc(gOverlay29Node8);
    gOverlay29Node0 = overlay29CreateReloc(gOverlay29Node4);
    gOverlay29NodeC = overlay29CreateReloc(gOverlay29Node8);
    overlay29CreateReloc(resource->last);
}
