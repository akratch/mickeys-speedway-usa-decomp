#include "PR/ultratypes.h"

/* DKR v77/v80 and JFG searches found no exact donor for this resource path. */

typedef struct Overlay68KindPair {
    s8 kind;
    s8 amount;
} Overlay68KindPair;

typedef struct Overlay68Probe {
    s32 assetOffsets[4];
    s32 assetSizes[4];
    s16 values[4];
} Overlay68Probe;

typedef struct Overlay68EntryHeader {
    s32 active;
    u8 field4;
    u8 timer;
    s16 index;
    s16 width;
    s16 generation;
    void *payload;
} Overlay68EntryHeader;

typedef struct Overlay68ResidentEntry {
    s32 thresholdNumerator;
    u8 pad04[0x1C];
} Overlay68ResidentEntry;

extern const Overlay68KindPair gOverlay68KindMap[];
extern Overlay68EntryHeader *gOverlay68SecondaryEntry;

extern s32 overlay68PayloadLimit(void);
extern void *overlay68AllocReloc(s32 size, s32 tag);
extern s32 overlay68RomLoadSectionReloc(u32 assetIndex, u32 address,
                                       s32 assetOffset, s32 size);
extern Overlay68ResidentEntry *overlay68GetResidentEntriesReloc(void);
extern s32 overlay68GetBlurEffectReloc(s32 kind);
extern void overlay68ReleaseReloc(void *resource);

/*
 * Policy-clean configured C plateaus at 17/122 positional words, with 120
 * emitted words, frame 0x38, and first mismatch +0x0. All 19 runtime roles
 * remain represented, but structural drift shifts sites after +0x64. All 119
 * flag configurations were nonexact; the closest debug family still differs
 * in 77 words and has the wrong extent. One codegen-faithful allocator trace
 * supported lexical stack-home scoping, which improved one word but did not
 * restore the target's 122-word/0x40-frame shape. Sentinel operand order was
 * byte-identical, so no batch was authorized. The assembly fallback remains;
 * ORT 1163's sole inbound is func_80004FE0+0x4C8.
 */
#ifdef NON_MATCHING
void overlay68RebuildSecondaryEntry(s32 kind) {
    s32 amount;
    const Overlay68KindPair *mapping;
    s32 currentKind;

    gOverlay68SecondaryEntry = 0;
    amount = -1;
    mapping = gOverlay68KindMap;

    if (mapping->kind != -1) {
        currentKind = mapping->kind;
        do {
            if (kind == currentKind) {
                amount = mapping->amount;
                break;
            }
            mapping++;
            currentKind = mapping->kind;
        } while (-1 != currentKind);
    }

    if (amount != -1) {
        Overlay68EntryHeader *entry;

        entry = overlay68AllocReloc(overlay68PayloadLimit(), 0x85);
        if (entry != 0) {
            Overlay68Probe *probe;

            probe = overlay68AllocReloc(sizeof(*probe), 0x85);
            if (probe != 0) {
                Overlay68ResidentEntry *entries;
                s32 threshold;
                s32 index;

                overlay68RomLoadSectionReloc(0x3F, (u32)probe,
                                             amount * (s32)sizeof(*probe),
                                             sizeof(*probe));
                entries = overlay68GetResidentEntriesReloc();
                index = overlay68GetBlurEffectReloc(kind);
                threshold = entries[index].thresholdNumerator / 5;
                if (threshold == 0) {
                    threshold = 0x7080;
                }

                index = 0;
                if ((probe->assetSizes[0] != 0) &&
                    (threshold < probe->values[0])) {
                    do {
                        index++;
                    } while ((index < 4) &&
                             (probe->assetSizes[index] != 0) &&
                             (threshold < probe->values[index]));
                }

                if (index >= 4) {
                    index = 3;
                } else if (probe->assetSizes[index] == 0) {
                    index--;
                }

                if (index >= 0) {
                    overlay68RomLoadSectionReloc(0x40, (u32)entry,
                                                 probe->assetOffsets[index],
                                                 probe->assetSizes[index]);
                    entry->payload = entry + 1;
                    gOverlay68SecondaryEntry = entry;
                }
                overlay68ReleaseReloc(probe);
            }
            if (gOverlay68SecondaryEntry == 0) {
                overlay68ReleaseReloc(entry);
            }
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o068/overlay68RebuildSecondaryEntry/func_overlay_068_F0001250_18C83B0.s")
#endif

/* PLATEAU-HANDOFF:overlay68RebuildSecondaryEntry:start
 * symbol: overlay68RebuildSecondaryEntry
 * score: 17/122 words
 * frame: 0x38
 * relocations: 19
 * first-mismatch: +0x0
 * summary: 120-word scoped C retains 19 roles; target requires 122 words and a 0x40 frame
 * PLATEAU-HANDOFF:overlay68RebuildSecondaryEntry:end
 */
