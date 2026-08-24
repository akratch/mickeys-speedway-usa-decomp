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

#ifdef NON_MATCHING
void overlay20UpdateObjectResource(Overlay20Object *object,
                                   Overlay20Config *config) {
    Overlay20Context *context;
    Overlay20Context *savedContext;
    Overlay20LookupResult *lookup;
    s32 baseX;
    s32 width;
    s32 height;
    s32 baseY;
    s32 objectY;
    void *owner;

    context = overlay20GetContextReloc();
    if ((config->useLookup != 0) &&
        ((savedContext = context),
         (lookup = overlay20LookupReloc(object->lookupIndex)),
         (context = savedContext), (lookup != 0))) {
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

    if ((config->entryIndex >= 0) &&
        (config->entryIndex < context->count)) {
        owner = context->entries[config->entryIndex * 2];
    } else {
        owner = *object->fallbackEntry;
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
