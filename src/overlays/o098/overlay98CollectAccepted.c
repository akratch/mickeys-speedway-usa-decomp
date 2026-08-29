#include "PR/ultratypes.h"

typedef struct O98Object {
    u8 pad00[0x8E];
    u8 accepted;
} O98Object;

typedef struct O98Entry {
    O98Object *object;
    f32 value;
} O98Entry;

extern u8 *overlay98AcquireContextReloc(void);
extern s32 overlay98CheckObject(O98Object *, u8 *, f32 *);
extern s32 gOverlay98AcceptedCount;
extern O98Entry gOverlay98AcceptedEntries[0x50];

/* Reproof of the former public claim: natural C requires frame/home and
 * schedule instruction edits to reproduce retail. Assembly is canonical. */
#ifdef NON_MATCHING
void overlay98CollectAccepted(s32 count, O98Object **objects) {
    f32 value;
    u8 *context;
    s32 index;

    context = overlay98AcquireContextReloc();
    gOverlay98AcceptedCount = 0;
    index = count - 1;
    if (index >= 0) {
        do {
            O98Object *object = objects[index];

            index--;
            if (overlay98CheckObject(object, context, &value) != 0) {
                O98Entry *entry;
                s32 next;

                object->accepted = 1;
                entry = &gOverlay98AcceptedEntries[gOverlay98AcceptedCount];
                next = gOverlay98AcceptedCount + 1;
                entry->object = object;
                gOverlay98AcceptedCount = next;
                entry->value = value;
                if (next >= 0x50) {
                    index = -1;
                }
            } else {
                object->accepted = 0;
            }
        } while (index >= 0);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o098/overlay98CollectAccepted/func_overlay_098_F0000144_18D8B04.s")
#endif
