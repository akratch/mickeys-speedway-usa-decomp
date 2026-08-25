#include "PR/ultratypes.h"

typedef struct Overlay84Choice {
    u8 pad00[0x11];
    u8 resource0;
    u8 pad12[2];
    u8 resource1;
    u8 pad15[5];
    u8 resource2;
    u8 resource3;
} Overlay84Choice;

typedef struct Overlay84Node {
    s16 angle;
    s16 tilt;
    s16 roll;
    u8 pad06[6];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x24];
    Overlay84Choice *choice;
} Overlay84Node;

typedef struct Overlay84ResourceRecord {
    s16 angle;
    s16 tilt;
    s16 roll;
    u8 pad06[6];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x14];
} Overlay84ResourceRecord;

typedef struct Overlay84Resource {
    u8 pad00[0x16];
    u8 flags;
    u8 recordIndex;
    u8 pad18[8];
    Overlay84ResourceRecord *records;
} Overlay84Resource;

typedef struct Overlay84State {
    u8 pad00;
    s8 current;
    u8 pad02;
    u8 timer;
    u8 phase;
    u8 pad05[4];
    u8 mode;
    u8 pad0A[6];
    s16 tilt;
    s16 targetTilt;
    s16 savedTilt;
    s16 currentTilt;
    u8 pad18[8];
    s32 angle;
    f32 height;
    f32 targetHeight;
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
    u8 pad3C[8];
    Overlay84Node *nodes[32];
    u8 selectedResource;
    u8 active;
} Overlay84State;

typedef struct Overlay84Object {
    u8 pad00[0x64];
    Overlay84State *state;
} Overlay84Object;

extern Overlay84Object *gOverlay84Object;
extern Overlay84Resource *overlay84LookupReloc(u8 resource);
extern void overlay84PrepareReloc(u8 resource);
extern void overlay84ReleaseReloc(u8 resource);

/* NON_MATCHING plateau (reconfirmed 2026-08-25): the nearest skeleton score is
 * 0.051 and all 119 flag combinations miss. The exact-size candidate differs
 * in nine of 101 words, first at +0x8: retail keeps the selected resource as a
 * full word in a2, uses a 0x30 frame, and spills it at sp+0x30, while natural
 * C truncates the u8 at entry, uses a 0x40 frame, and spills a byte at sp+0x27.
 * s32/u32 locals or reusing kind expand the mismatch to 69 words; u8 parameter
 * forms add eight bytes. A bounded two-worker permuter chose the same int-local
 * dead end and found no exact form. */
#ifdef NON_MATCHING
void overlay84ActivateCurrent(s32 kind) {
    Overlay84Object *object;
    Overlay84State *state;
    Overlay84Node *node;
    Overlay84Choice *choice;
    Overlay84Resource *resource;
    Overlay84ResourceRecord *record;
    u8 selected;
    s16 tilt;
    f32 height;

    selected = kind;
    object = gOverlay84Object;
    if (object != NULL) {
        state = object->state;
        node = state->nodes[state->current];
        state->active = 0;
        choice = node->choice;

        switch (kind) {
        case 0:
            selected = choice->resource0;
            break;
        case 1:
            selected = choice->resource1;
            break;
        case 2:
            selected = choice->resource2;
            break;
        case 3:
            selected = choice->resource3;
            break;
        }

        resource = overlay84LookupReloc(selected);
        if (resource != NULL) {
            record = &resource->records[resource->recordIndex - 1];
            record->x = node->x;
            record->y = node->y;
            record->z = node->z;
            record->angle = node->angle;
            record->tilt = node->tilt;
            record->roll = node->roll;

            state->selectedResource = selected;
            state->phase = 0;
            state->timer = 20;
            state->scale = 1.0f;
            tilt = -node->tilt;
            state->tilt = tilt;
            state->targetTilt = tilt;
            state->savedTilt = state->currentTilt;
            state->angle = 0x8000 - node->angle;
            height = node->y;
            state->height = height;
            state->targetHeight = height;
            state->x = node->x;
            state->y = node->y;
            state->z = node->z;

            overlay84PrepareReloc(selected);
            overlay84ReleaseReloc(state->selectedResource);
            resource->flags |= 2;
        }
        state->mode = 1;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o084/overlay84ActivateCurrent/func_overlay_084_F0001060_18D1540.s")
#endif
