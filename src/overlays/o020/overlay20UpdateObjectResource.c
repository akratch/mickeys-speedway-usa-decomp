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

/* Bounded reproof (2026-08-29): configured full-TU C is exact-sized at 98
 * words with frame 0x70 and matches 90/98 raw/normalized, first +0xB0.
 * All four candidate R_MIPS_26 records are exact by offset/type at +0x14,
 * +0x30, +0x134, and +0x168. Runtime tables authenticate their identities as
 * trackGetTrack, func_8000FEEC, local overlay20ConfigureResource, and sqrtf;
 * the assembly target's zero-valued carrier does not encode those identities.
 * The owned +0x204..+0x38C / ROM 0x18767DC..0x1876964 range has no target
 * padding; IDO's trailing eight-byte section alignment is outside the function.
 * ORT 1226 and the sole resident inbound are authenticated independently.
 *
 * The complete 119-configuration lattice is nonexact; canonical -O2 -mips2
 * and six equivalent rows tie for best. One stock-fidelity instrumented compile
 * emitted text identical to the normal compiler while retaining both allocator
 * and UGEN FIFO traces. The trace shows one pool-to-temp transition: the scoped
 * count carrier colors v1, while the target routes that load through t6. The
 * later value0F/start t6/t7 exchange is the downstream free-list consequence,
 * not an independent web with a separate source lever.
 * Direct field access removes the colored count but regresses to 73/98; prior
 * forced splitting is also known to grow the function to 100 words and frame
 * 0x78. With no natural strict gain, combination and the gain-gated permutation
 * batch are ineligible. Preserve this body and fallback until a new legal IDO
 * pool-to-temp web-formation mechanism exists. Linked equality and the exact
 * full ROM continue to prove the assembly fallback only. Pinned DKR v77/v80/JFG
 * and function-specific structural scans remain negative. */
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
