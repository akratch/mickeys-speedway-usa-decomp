#include "PR/ultratypes.h"

typedef struct Overlay20Entry {
    f32 x;
    f32 y;
    f32 radius;
    f32 radiusSquared;
    s16 minX;
    s16 minY;
    s16 maxX;
    s16 maxY;
    s16 id;
    s16 lifetime;
    f32 scale;
    f32 radiusRatio;
} Overlay20Entry;

extern s32 gOverlay20EntryCount;
extern u32 gOverlay20ActiveBits;
extern Overlay20Entry gOverlay20Pool[];
extern Overlay20Entry *gOverlay20Entries[];
extern f32 gOverlay20LifetimeScale;

/* DKR v77/v80 and JFG have no exact donor for this entry allocator. */
#ifdef NON_MATCHING
Overlay20Entry *overlay20ConfigureEntry(Overlay20Entry *entry, f32 x, f32 y,
                                        f32 radius, s32 id, f32 lifetime,
                                        f32 scale, f32 ratio) {
    s32 index;
    u32 bit;

    if (entry == NULL) {
        index = gOverlay20EntryCount;
        bit = 1;
        if (index >= 32 || gOverlay20ActiveBits == -1) {
            entry = NULL;
        } else {
            entry = gOverlay20Pool;
            if (gOverlay20ActiveBits & 1) {
                do {
                    bit <<= 1;
                    entry++;
                } while (gOverlay20ActiveBits & bit);
            }
            gOverlay20Entries[index] = entry;
            gOverlay20EntryCount = index + 1;
            gOverlay20ActiveBits |= bit;
        }
    } else {
        id = entry->id;
    }

    if (entry != NULL) {
        entry->x = x;
        entry->y = y;
        entry->radius = radius;
        entry->radiusSquared = radius * radius;
        entry->minX = (s32)(x - radius);
        entry->minY = (s32)(y - radius);
        entry->maxX = (s32)(x + radius);
        entry->maxY = (s32)(y + radius);
        entry->id = id;
        entry->lifetime = (s32)(lifetime * gOverlay20LifetimeScale);
        entry->scale = 65536.0f / scale;
        entry->radiusRatio = ratio / radius;
    }
    return entry;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o020/overlay20ConfigureEntry/func_overlay_020_F0000E28_1877400.s")
#endif
