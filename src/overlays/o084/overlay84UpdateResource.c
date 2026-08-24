#include "PR/ultratypes.h"

typedef struct Overlay84State {
    u8 pad0[9];
    u8 mode;
    u8 resource;
    u8 padB[0xBA];
    u8 active;
    u8 marked;
} Overlay84State;

typedef struct Overlay84Resource {
    u8 pad0[0x1C];
    f32 scale;
} Overlay84Resource;

extern u32 gOverlay84InputFlags;
extern Overlay84Resource *overlay84GetResource(u8 resource);
extern void overlay84ConfigureResource(Overlay84Resource *resource, f32 scale,
                                       s32 arg2, s32 arg3);
extern void overlay84ReleaseResource(u8 resource);

/* DKR v77/v80 have no matching input-gated resource update. */
void overlay84UpdateResource(void *unused, Overlay84State *state, s32 value) {
    Overlay84Resource *resource;

    if ((gOverlay84InputFlags & 0xD000) != 0 && state->active != 0 &&
        state->marked == 0) {
        resource = overlay84GetResource(state->resource);
        if (resource != 0) {
            resource->scale = 1.0f;
            overlay84ConfigureResource(resource, (f32)value, value, value);
        }
    }
    resource = overlay84GetResource(state->resource);
    if (resource != 0) {
        state->mode = 0;
        overlay84ReleaseResource(state->resource);
    }
}
