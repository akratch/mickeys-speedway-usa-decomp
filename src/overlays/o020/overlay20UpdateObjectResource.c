#include "PR/ultratypes.h"

typedef struct Overlay20LookupResult {
    s16 x0;
    s16 pad02;
    s16 y0;
    s16 x1;
    s16 pad08;
    s16 y1;
} Overlay20LookupResult;

typedef struct Overlay20Context {
    void **entries;
    u8 pad04[0x14];
    s16 count;
} Overlay20Context;

typedef struct Overlay20Config {
    u8 pad00[0xA];
    s8 useLookup;
    s8 entryIndex;
    u8 columns;
    u8 rows;
    u8 value0E;
    u8 value0F;
    s16 width;
    s16 height;
    u8 start;
    u8 current;
    u8 end;
    u8 scaleDivisor;
} Overlay20Config;

typedef struct Overlay20Object {
    u8 pad00[0xC];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x16];
    s16 lookupIndex;
    u8 pad30[4];
    f32 radius;
    u8 pad38[0x30];
    void **fallbackEntry;
    u8 pad6C[0x18];
    void *resource;
} Overlay20Object;

extern Overlay20Context *overlay20GetContextReloc(void);
extern Overlay20LookupResult *overlay20LookupReloc(s16 index);
extern void *overlay20ConfigureResourceReloc();
extern f32 overlay20SqrtReloc(f32 value);

/*
 * Plateau (2026-08-25, 10 source attempts plus a bounded permuter batch):
 * the best -O2 candidate has the exact 98-word size, differs in 8 words, and
 * agrees through +0xAC before first diverging at +0xB0.  A block-scoped s16
 * count recovers the fallback pointer, index scaling, address calculation,
 * and long-call argument web; IDO still colors that count and the entries
 * pointer one register apart from the target.  The full flag lattice found no
 * better group, and hoisting the count assignment loses both size and CFG.
 */
/*
 * Current-run plateau (2026-08-25): the 119-group sweep again selected
 * -O2 -mips2 at exact size, with eight masked words different from +0xB0.
 * Direct field access regressed to 25 words; moving the count or pointer
 * declarations broke the earlier exact prefix.  A two-worker, ten-minute
 * permuter batch found no improvement over base score 60.  The remaining
 * blocker is the count/entries temporary coloring only.
 */
#ifdef NON_MATCHING
void overlay20UpdateObjectResource(Overlay20Object *object,
                                   Overlay20Config *config) {
    s32 baseX;
    s32 baseY;
    s32 objectY;
    s32 width;
    s32 height;
    void *owner;
    Overlay20LookupResult *lookup;
    Overlay20Context *context;

    context = overlay20GetContextReloc();
    if ((config->useLookup != 0) &&
        ((lookup = overlay20LookupReloc(object->lookupIndex)), lookup != 0)) {
        baseX = lookup->x0;
        objectY = (s32)object->y;
        baseY = lookup->y0;
        width = lookup->x1 - lookup->x0;
        height = lookup->y1 - lookup->y0;
    } else {
        baseX = (s32)object->x;
        objectY = (s32)object->y;
        baseY = (s32)object->z;
        width = config->width;
        height = config->height;
    }

    {
        s16 count;

        if ((config->entryIndex >= 0) &&
            (config->entryIndex < (count = context->count))) {
            owner = context->entries[config->entryIndex * 2];
        } else {
            owner = *object->fallbackEntry;
        }
    }

    object->resource = overlay20ConfigureResourceReloc(
        object->resource, baseX, objectY, baseY, width, height,
        config->columns, config->rows, owner, config->value0E,
        config->value0F, config->start, config->current, config->end,
        config->scaleDivisor);
    object->radius = overlay20SqrtReloc((f32)((width * width) +
                                               (height * height)));
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o020/overlay20UpdateObjectResource/func_overlay_020_F0000204_18767DC.s")
#endif
