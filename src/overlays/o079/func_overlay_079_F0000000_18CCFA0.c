#include "PR/ultratypes.h"

typedef struct Overlay79InitState {
    u8 pad00[0xC];
    s32 field0C;
    f32 value10;
    f32 value14;
    u8 pad18[8];
    f32 scaled20;
    f32 position24;
    f32 position28;
    f32 position2C;
    f32 squared30;
    f32 range34;
} Overlay79InitState;

typedef struct Overlay79InitObject {
    s16 angle;
    u8 pad02[6];
    f32 scaled08;
    f32 position0C;
    f32 position10;
    f32 position14;
    u8 pad18[0x28];
    f32 *scale40;
    u8 pad44[0x20];
    Overlay79InitState *state64;
} Overlay79InitObject;

typedef struct Overlay79InitConfig {
    u8 pad00[0xA];
    s16 angle0A;
    s16 scale0C;
    s16 range0E;
} Overlay79InitConfig;

extern f32 gOverlay79InitScaleReloc;
extern s32 mathRnd(s32 lower, s32 upper);
extern f32 func_8002A8C0(s32 angle);
extern f32 func_8002A8BC(s32 angle);
extern void func_8005AD64(Overlay79InitObject *object, s32 mode, s32 index,
                          f32 value);

/*
 * Plateau (2026-08-25, 7 attempts): the canonical -O2 candidate has the
 * exact 77-word size, differs in 10 words, and first diverges at +0x48.
 * The remaining delta begins with a commuted floating-point multiply and
 * propagates through the later floating-point register choices.  All source
 * associations and the complete flag lattice retain that allocation split.
 * Revalidated on 2026-08-25: the full 119-combination lattice reproduced the
 * same result, and a 10-minute two-worker permuter batch found no improvement
 * over its base score of 105.
 */
#ifdef NON_MATCHING
void func_overlay_079_F0000000_18CCFA0(Overlay79InitObject *object,
                                        Overlay79InitConfig *config,
                                        void *unused) {
    Overlay79InitState *state;
    f32 range;
    s32 random;

    (void)unused;
    state = object->state64;
    object->scaled08 = config->scale0C * gOverlay79InitScaleReloc
                     * *object->scale40;
    object->angle = config->angle0A;
    state->scaled20 = object->scaled08 * 60.0f;
    state->range34 = config->range0E;
    state->squared30 = state->range34 * state->range34;
    state->position24 = object->position0C;
    state->position28 = object->position10;
    state->position2C = object->position14;
    state->field0C = 0xF0;
    random = mathRnd(-0x7FFF, 0x8000);
    range = mathRnd(0, (s32)state->range34);
    state->value10 = state->position24 - (func_8002A8C0(random) * range);
    state->value14 = state->position2C - (func_8002A8BC(random) * range);
    func_8005AD64(object, 0, -1, 0.0f);
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o079/func_overlay_079_F0000000_18CCFA0/func_overlay_079_F0000000_18CCFA0.s")
#endif
