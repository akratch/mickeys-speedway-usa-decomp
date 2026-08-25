/*
 * Sprite/object animation controls -- ROM 0x1BEA0-0x1C790.
 *
 * PROVENANCE -- the TU and seven descriptive function names are borrowed
 * from Jet Force Gemini's public retail-derived src/spranim.c and its
 * nonmatching assembly names.  The attribution is supported at tier B by
 * object-control call roles and at tier D by function order and masked
 * instruction shape.  No JFG body is adapted by this scaffold.
 */

#include "PR/ultratypes.h"

typedef struct SpranimBAE4Target {
    u8 pad0[0x132];
    s16 state132;
} SpranimBAE4Target;

typedef struct SpranimBAE4Object {
    u8 pad0[0x58];
    SpranimBAE4Target *target58;
} SpranimBAE4Object;

typedef struct SpranimBB10Header {
    u8 pad0[0x22];
    s8 count22;
} SpranimBB10Header;

typedef struct SpranimBB10Object {
    u8 pad0[0x3A];
    s8 index3A;
    u8 pad3B[5];
    SpranimBB10Header *header40;
    u8 pad44[0x24];
    void **entries68;
    u8 pad6C[0x1C];
    u32 flags88;
} SpranimBB10Object;

typedef struct SprasjiInitState {
    u8 pad0[8];
    f32 scale;
    u8 padC[0x34];
    f32 *baseScale;
} SprasjiInitState;

typedef struct SprasjiInitEntry {
    u8 pad0[0xB];
    u8 scale;
} SprasjiInitEntry;

typedef struct SpranimInitState {
    u8 pad0[8];
    f32 scale;
    u8 padC[0x1C];
    f32 initialValue;
    u8 pad2C[0x14];
    f32 *baseScale;
    u8 pad44[0x40];
    s32 animationId;
} SpranimInitState;

typedef struct SpranimInitEntry {
    u8 pad0[0xA];
    u8 animationId;
    u8 scale;
    u8 initialValue;
} SpranimInitEntry;

typedef struct SpranimControlState {
    u8 pad0[0x28];
    u8 animationState[0x40];
    void **entries;
    u8 pad6C[0x18];
    s32 animationId;
} SpranimControlState;

typedef struct TexscrollEntry {
    s16 textureIndex;
    s16 pad2;
    s16 speedX;
    s16 speedY;
    s16 offsetX;
    s16 offsetY;
} TexscrollEntry;

typedef struct TexscrollState {
    u8 pad0[0x64];
    TexscrollEntry *entry;
} TexscrollState;

typedef struct SpranimOnceState {
    u8 pad0[0x28];
    f32 value;
    u8 pad2C[0x3C];
    void **entries;
    u8 pad6C[0x18];
    s32 animationId;
} SpranimOnceState;

typedef struct RangetriggerEntry {
    u8 pad0[0xA];
    u16 radius;
    u16 triggerId;
} RangetriggerEntry;

typedef struct RangetriggerState {
    u8 pad0[0xC];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x24];
    RangetriggerEntry *entry;
    u8 pad40[0x40];
    s32 activeTrigger;
} RangetriggerState;

extern u8 D_8007BF2C;
extern void func_80006EA0(void *object);
extern void func_80020D8C(void *arg0, s32 arg1, s32 arg2, void *arg3);
extern void func_8000D16C(s16 textureIndex, s32 x, s32 y, s32 updateRate);
extern void func_80036544(void *entry, s32 *mode, s32 animationId, void *state, s32 updateRate);
extern s32 func_8005776C(f32 x, f32 y, f32 z, f32 radius, s32 mode, void *hits);
extern void partUpdateTriggers(void *state, s32 updateRate);

/* PROVENANCE -- adapted from JFG's public asm/nonmatchings/spranim/spranimInit.s, with Mickey's offsets. */
void spranimInit(SpranimInitState *state, SpranimInitEntry *entry) {
    f32 scale;

    scale = (s32) (entry->scale & 0xFF);
    if (scale < 10.0f) {
        scale = 10.0f;
    }
    scale /= 64;
    state->scale = *state->baseScale * scale;
    state->animationId = entry->animationId;
    state->initialValue = entry->initialValue;
}
/* PROVENANCE -- adapted from JFG's public asm/nonmatchings/spranim/spranimControl.s, with Mickey's offsets. */
void spranimControl(SpranimControlState *state, s32 updateRate) {
    s32 mode;

    mode = 9;
    func_80036544(*state->entries, &mode, state->animationId, state->animationState, updateRate);
}
void sprasjiInit(SprasjiInitState *state, SprasjiInitEntry *entry) {
    f32 scale;

    scale = (s32) (entry->scale & 0xFF);
    if (scale < 10.0f) {
        scale = 10.0f;
    }
    scale /= 64;
    state->scale = *state->baseScale * scale;
}
/* PROVENANCE -- adapted from JFG's public asm/nonmatchings/spranim/spranimOnceControl.s, with Mickey's offsets. */
void spranimOnceControl(SpranimOnceState *state, s32 updateRate) {
    s32 mode[2];
    f32 initialValue;
    void *entry;

    mode[1] = 9;
    entry = *state->entries;
    initialValue = state->value;
    func_80036544(entry, &mode[1], state->animationId, &state->value, updateRate);
    if (state->value < initialValue) {
        func_80006EA0(state);
    }
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/spranim/effectboxControl.s")
/* PROVENANCE -- adapted from JFG's public asm/nonmatchings/spranim/texscrollControl.s, with Mickey's object offset. */
void texscrollControl(TexscrollState *state, s32 updateRate) {
    s32 x;
    s32 y;
    TexscrollEntry *entry;

    entry = state->entry;
    x = updateRate;
    x = entry->speedX * x;
    x += entry->offsetX;
    entry->offsetX = x & 3;
    x >>= 2;
    y = entry->speedY * updateRate;
    y += entry->offsetY;
    entry->offsetY = y & 3;
    y >>= 2;
    func_8000D16C(entry->textureIndex, x, y, updateRate);
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/spranim/func_8001B798.s")
#ifdef NON_MATCHING
/* PROVENANCE -- adapted from JFG's public asm/nonmatchings/spranim/rangetriggerControl.s, with Mickey's offsets. */
void rangetriggerControl(RangetriggerState *state, s32 updateRate) {
    RangetriggerEntry *entry;
    u64 hits[4];

    entry = state->entry;
    if (func_8005776C(state->x, state->y, state->z, entry->radius, 1, hits) > 0) {
        state->activeTrigger = entry->triggerId;
    } else {
        state->activeTrigger = 0;
    }
    partUpdateTriggers(state, updateRate);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/spranim/rangetriggerControl.s")
#endif
void func_8001BAE4(SpranimBAE4Object *arg0, void *arg1) {
    arg0->target58->state132 = 1;
}
void func_8001BAF8(void *arg0, void *arg1) {
}
void func_8001BB04(void *arg0, void *arg1) {
}
void func_8001BB10(SpranimBB10Object *arg0, void *arg1) {
    s8 index;
    s32 frame;

    arg0->index3A = D_8007BF2C;
    index = arg0->index3A;
    if ((index < 0) || (index >= arg0->header40->count22)) {
        arg0->index3A = 0;
        index = arg0->index3A;
    }
    frame = (arg0->flags88 & 3) << 8;
    func_80020D8C(arg0->entries68[index], 0, frame, arg0);
}
