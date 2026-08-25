#include "PR/ultratypes.h"
typedef struct State { u8 pad0[4]; u16 flags; } State;
typedef struct Object {
    u8 pad00[0xC]; f32 x; f32 y; f32 z; u8 pad18[0x2E]; s16 type;
    u8 pad48[0x1C]; State *state;
} Object;
extern Object **overlay3GetObjectRangeReloc(s32 *, s32 *);
extern s32 overlay3ContainsValueReloc(Object *, Object *);
extern f32 overlay3DistanceSquaredReloc(f32, f32, f32, f32, f32, f32);
extern f32 gOverlay3SearchMaxDistance[];
/*
 * Plateau (2026-08-25, r4 pass): the 77-word candidate retains exact size,
 * control flow, and relocation surface, with four non-relocation differences
 * first at +0x40.  The 119-combination flag lattice was neutral.  Ten directed
 * variants using dead-web priority/phantom-pop forms, output-helper prototype
 * variants, and index qualification either reproduced this object or worsened
 * it.  The likely missing structure is an original local/live range that makes
 * IDO load the start index into a0 instead of v1.
 */
#ifdef NON_MATCHING
Object *overlay3FindClosestObject(Object *anchor, void *unused) {
    s32 pad;
    s32 start; s32 end;
    f32 distance; f32 bestDistance;
    Object **objects; Object **cursor; Object *object; Object *best;
    State *state; s32 index;
    if (&pad);
    objects = overlay3GetObjectRangeReloc(&start, &end);
    bestDistance = gOverlay3SearchMaxDistance[3];
    best = 0; index = start;
    if (start < end) {
        cursor = objects;
        cursor += start;
        do {
            object = *cursor;
            if (object->type == 0x86 || object->type == 0xEB) {
                state = object->state;
                if (overlay3ContainsValueReloc(anchor, object) == 0 && state->flags == 0) {
                    distance = overlay3DistanceSquaredReloc(object->x, object->y, object->z,
                                                              anchor->x, anchor->y, anchor->z);
                    if (distance < bestDistance) { bestDistance = distance; best = object; }
                }
            }
            index++; cursor++;
        } while (index < end);
    }
    return best;
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o003/overlay3FindClosestObject/func_overlay_003_F000027C_1859FAC.s")
#endif
