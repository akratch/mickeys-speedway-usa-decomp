#include "PR/ultratypes.h"

/* DKR v77/v80 and JFG searches found no exact donor for this resource path. */

typedef struct Overlay68KindPair {
    s8 kind;
    s8 amount;
} Overlay68KindPair;

typedef struct Overlay68Probe {
    void *data[4];
    void *slots[4];
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

extern const Overlay68KindPair gOverlay68KindMapInitial[];
extern volatile const s8 gOverlay68KindMapLoop;
extern Overlay68EntryHeader *gOverlay68SecondaryEntry;

extern s32 overlay68PayloadLimit(void);
extern Overlay68EntryHeader *overlay68AllocReloc(s32 size, s32 tag);
extern Overlay68Probe *overlay68AllocProbeReloc(s32 size, s32 tag);
extern void overlay68FillProbeReloc(s32 kind, Overlay68Probe *probe,
                                    s32 totalBytes, s32 stride);
extern Overlay68ResidentEntry *overlay68GetResidentEntriesReloc(void);
extern s32 overlay68MapResidentIndexReloc(s32 kind);
extern void overlay68BindEntryReloc(s32 kind, Overlay68EntryHeader *entry,
                                    void *data, void *slot);
extern void overlay68FreeProbeReloc(void *probe);
extern void overlay68ReleaseReloc(void *resource);

#ifdef NON_MATCHING
void overlay68RebuildSecondaryEntry(s32 kind) {
    const Overlay68KindPair *mapping;
    volatile const s8 *loopMapping;
    Overlay68EntryHeader *entry;
    Overlay68Probe *probe;
    Overlay68ResidentEntry *entries;
    s16 *valueCursor;
    s32 currentKind;
    s32 amount;
    s32 threshold;
    s32 index;

    gOverlay68SecondaryEntry = 0;
    amount = -1;
    mapping = gOverlay68KindMapInitial;

    if (mapping->kind != -1) {
        loopMapping = &gOverlay68KindMapLoop;
        currentKind = *loopMapping;
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
        entry = overlay68AllocReloc(overlay68PayloadLimit(), 0x85);
        if (entry != 0) {
            probe = overlay68AllocProbeReloc(sizeof(*probe), 0x85);
            if (probe != 0) {
                overlay68FillProbeReloc(0x3F, probe,
                                        amount * (s32)sizeof(*probe),
                                        sizeof(*probe));
                entries = overlay68GetResidentEntriesReloc();
                index = overlay68MapResidentIndexReloc(kind);
                threshold = entries[index].thresholdNumerator / 5;
                if (threshold == 0) {
                    threshold = 0x7080;
                }

                index = 0;
                valueCursor = (s16 *)probe;
                if ((probe->slots[0] != 0) &&
                    (threshold < valueCursor[0x10])) {
                    do {
                        index++;
                    } while ((index < 4) &&
                             (probe->slots[index] != 0) &&
                             (threshold < valueCursor[index + 0x10]));
                }

                if (index >= 4) {
                    index = 3;
                } else if (probe->slots[index] == 0) {
                    index--;
                }

                if (index >= 0) {
                    overlay68BindEntryReloc(0x40, entry,
                                            probe->data[index],
                                            probe->slots[index]);
                    entry->payload = entry + 1;
                    gOverlay68SecondaryEntry = entry;
                }
                overlay68FreeProbeReloc(probe);
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
