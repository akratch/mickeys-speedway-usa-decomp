#include "PR/ultratypes.h"
typedef struct Overlay3State { u8 pad000[1]; s8 selector; u8 pad002[0x3B4]; s16 scoreHigh; s16 scoreLow; u8 pad3BA[0x40]; s16 blocked; } Overlay3State;
typedef struct Overlay3Object { u8 pad00[0x0C]; f32 x; f32 y; f32 z; u8 pad18[0x4C]; Overlay3State *state; } Overlay3Object;
typedef struct Overlay3Search { u8 pad000[0x38D]; u8 cachedIndex; u16 timer; u8 pad390[0x18]; u8 weights[1]; } Overlay3Search;
extern Overlay3Object **overlay3GetSearchObjectsReloc(s32 *count);
extern s32 overlay3ContainsValueReloc(Overlay3Object *anchor, Overlay3Object *object);
extern s32 overlay3RandomRangeReloc(s32 low, s32 high);
extern f32 overlay3SqrtReloc(f32 value);
#ifdef NON_MATCHING
Overlay3Object *overlay3SelectScoredObject(Overlay3Object *anchor, Overlay3Search *search, s32 elapsed) {
    s32 count; Overlay3Object *result; s32 bestIndex; Overlay3Object **objects;
    Overlay3Object **cursor; Overlay3Object *object; Overlay3State *state;
    s32 index; s32 bestScore; s32 score; f32 dx; f32 dz; u16 timer; u8 cachedIndex;
    objects = overlay3GetSearchObjectsReloc(&count);
    result = 0;
    timer = search->timer;
    if ((elapsed < timer) && ((cachedIndex = search->cachedIndex) != 0x7F)) {
        search->timer = timer - elapsed;
        result = objects[cachedIndex];
    } else {
        bestScore = -1000000;
        index = count - 1;
        if (count != 0) {
            cursor = &objects[index];
            do {
                object = *cursor;
                if ((object != anchor) && (object->state->blocked == 0) &&
                    (overlay3ContainsValueReloc(anchor, object) == 0)) {
                    score = overlay3RandomRangeReloc(0, 2000);
                    state = object->state;
                    score += (state->scoreHigh - state->scoreLow) * 1000;
                    dx = anchor->x - object->x;
                    dz = anchor->z - object->z;
                    score -= ((s32)overlay3SqrtReloc((dx * dx) + (dz * dz))) / 8;
                    score += search->weights[state->selector] << 8;
                    if (bestScore < score) {
                        bestScore = score;
                        bestIndex = index;
                        result = object;
                    }
                }
                cursor--;
            } while (index--);
        }
        if (result != 0) {
            search->cachedIndex = bestIndex;
            search->timer = overlay3RandomRangeReloc(3, 6) * 60;
        }
    }
    return result;
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o003/overlay3SelectScoredObject/func_overlay_003_F00003B0_185A0E0.s")
#endif
