#include "PR/ultratypes.h"

/*
 * Plateau (2026-08-28): both bodies are 0xF8 bytes (62 words) with a 0x28
 * frame. Ten words differ first at +0x10; after separating the table-address
 * relocation, nine instruction words remain in the descriptor/call-argument
 * register web. The candidate has the table HI/LO pair plus four call relocs;
 * the synthetic target object exposes only the four collapsed call relocs.
 * Six natural lookup, handle, descriptor, and result-scope forms either
 * reproduced this baseline or regressed/gained two words. A bounded permuter
 * import could not join the address-derived assembly label to the friendly C
 * name, so it produced no additional candidate.
 */

typedef struct Overlay59Descriptor {
    u32 first;
    u32 second;
    u8 pad08[8];
} Overlay59Descriptor;

typedef struct Overlay59Entry {
    u8 pad00[0x20];
    Overlay59Descriptor *owner;
    u32 handles[8];
} Overlay59Entry;

typedef struct Overlay59DescriptorGroup {
    Overlay59Descriptor **descriptors;
    u32 unused;
} Overlay59DescriptorGroup;

extern Overlay59DescriptorGroup gOverlay59DescriptorTables[];
extern void overlay59ReleaseEntryReloc(Overlay59Entry *entry);
extern u32 overlay59AcquireReloc(u32 value);

#ifdef NON_MATCHING
s32 overlay59PrepareEntry(Overlay59Entry *entry, s32 tableIndex, s32 itemIndex) {
    Overlay59Descriptor *descriptor;
    u32 handle;
    s32 count;
    s32 result;

    descriptor = gOverlay59DescriptorTables[tableIndex].descriptors[itemIndex];
    result = 1;
    if (descriptor != entry->owner) {
        overlay59ReleaseEntryReloc(entry);
        entry->owner = descriptor;
        count = 0;
        if ((handle = descriptor->first) != 0) {
            do {
                if ((handle = overlay59AcquireReloc(handle)) == 0) {
                    result = 0;
                } else {
                    entry->handles[count] = handle;
                    count++;
                }

                if ((handle = descriptor->second) != 0) {
                    if ((handle = overlay59AcquireReloc(handle)) == 0) {
                        result = 0;
                    } else {
                        entry->handles[count] = handle;
                        count++;
                    }
                }
                descriptor++;
            } while ((handle = descriptor->first) != 0);
        }

        if (result == 0) {
            overlay59ReleaseEntryReloc(entry);
        }
    }
    return result;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o059/overlay59PrepareEntry/func_overlay_059_F0000070_18B87C0.s")
#endif
