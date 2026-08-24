#include "PR/ultratypes.h"

typedef struct Overlay14Asset {
    s16 count;
    s16 pad2;
    void *pointer;
    s32 pad8;
    s16 marker;
    s16 padE;
} Overlay14Asset;

extern s32 *gOverlay14Ranges;
extern s32 gOverlay14DefaultA4;
extern s32 gOverlay14DefaultB4;
extern void *overlay14AssetCall();

#ifdef NON_MATCHING
Overlay14Asset *func_overlay_014_F00009F4_18702CC(s32 index, s32 context) {
    s32 start;
    s32 size;
    Overlay14Asset *asset;
    s32 i;
    Overlay14Asset *entry;

    start = gOverlay14Ranges[index];
    size = gOverlay14Ranges[index + 1] - start;
    asset = (Overlay14Asset *)overlay14AssetCall(size, 0x85, start);
    if (asset != 0) {
        overlay14AssetCall(context, asset, start, size);
        i = 0;
        entry = asset;
        if (asset->count > 0) {
            do {
                if (entry->pointer == 0) {
                    if (entry->marker == 0x4000) {
                        entry->pointer = &gOverlay14DefaultA4;
                    } else {
                        entry->pointer = &gOverlay14DefaultB4;
                    }
                } else {
                    entry->pointer = (u8 *)asset + (s32)entry->pointer;
                }
                i++;
                entry++;
            } while (i < asset->count);
        }
    }
    return asset;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o014/func_overlay_014_F00009F4_18702CC/func_overlay_014_F00009F4_18702CC.s")
#endif
