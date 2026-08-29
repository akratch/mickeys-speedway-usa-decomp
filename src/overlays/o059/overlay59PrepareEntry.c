#include "PR/ultratypes.h"

/*
 * PROVENANCE: Mickey-derived from this overlay's owned assembly and runtime
 * relocation tables; pinned DKR/JFG scans found no donor.
 *
 * Bounded reproof (2026-08-29): the policy-clean split-carrier C compiles to
 * the exact 0xF8/62-word extent with frame 0x28. It matches 52/62 raw words
 * and 53/62 relocation-normalized words, with first raw +0x10 and first
 * substantive +0x54. Ten raw sites remain; normalizing the table LO16 leaves
 * nine sites in one descriptor/call-argument carrier web. All 119 compiler
 * configurations were nonexact, with canonical -O2 -mips2 tied for best. One
 * instrumented UOPT trace found seven colored webs and an otherwise exact
 * temporary-register lane. The trace-supported natural form that coalesced
 * descriptor input with the acquisition result compiled byte-identically to
 * this split-carrier body, so there was no strict gain to retain.
 * The owned range is overlay +0x70..+0x168 (ROM 0x18B87C0..0x18B88B8), with
 * no target padding and the next function beginning at +0x168. The LOCAL table
 * pair and four call records are authenticated by runtime metadata; the
 * synthetic target collapses the zero-field calls and cannot prove identities.
 * Four local callers use the s32 three-argument ABI. The fallback remains
 * active; the next pass needs a new source-faithful carrier/coalescing lever.
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
extern void overlay59PrepareReleaseReloc(Overlay59Entry *entry);
extern u32 overlay59PrepareAcquireReloc(u32 value);

#ifdef NON_MATCHING
/* PLATEAU-HANDOFF
 * symbol: overlay59PrepareEntry
 * score: 53/62 words
 * frame: 0x28
 * relocations: 6
 * first-mismatch: +0x54
 * summary: All 119 flags were nonexact; trace-supported carrier coalescing was flat, leaving one descriptor/call-argument carrier web.
 */
s32 overlay59PrepareEntry(Overlay59Entry *entry, s32 tableIndex, s32 itemIndex) {
    Overlay59Descriptor *descriptor;
    u32 descriptorValue;
    u32 handle;
    s32 count;
    s32 result;

    descriptor = gOverlay59DescriptorTables[tableIndex].descriptors[itemIndex];
    result = 1;
    if (descriptor != entry->owner) {
        overlay59PrepareReleaseReloc(entry);
        entry->owner = descriptor;
        count = 0;
        descriptorValue = descriptor->first;
        if (descriptorValue != 0) {
            do {
                handle = overlay59PrepareAcquireReloc(descriptorValue);
                if (handle == 0) {
                    result = 0;
                } else {
                    entry->handles[count] = handle;
                    count++;
                }

                descriptorValue = descriptor->second;
                if (descriptorValue != 0) {
                    handle = overlay59PrepareAcquireReloc(descriptorValue);
                    if (handle == 0) {
                        result = 0;
                    } else {
                        entry->handles[count] = handle;
                        count++;
                    }
                }
                descriptor++;
                descriptorValue = descriptor->first;
            } while (descriptorValue != 0);
        }

        if (result == 0) {
            overlay59PrepareReleaseReloc(entry);
        }
    }
    return result;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o059/overlay59PrepareEntry/func_overlay_059_F0000070_18B87C0.s")
#endif
