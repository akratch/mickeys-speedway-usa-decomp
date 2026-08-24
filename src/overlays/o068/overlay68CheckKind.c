#include "PR/ultratypes.h"

/* DKR v77/v80 and JFG searches found no exact donor for this probe check. */

typedef struct Overlay68KindPair {
    s8 kind;
    s8 amount;
} Overlay68KindPair;

typedef struct Overlay68Probe {
    u8 pad00[0x10];
    void *slots[4];
    s16 values[4];
} Overlay68Probe;

typedef struct Overlay68ResidentEntry {
    s32 thresholdNumerator;
    u8 pad04[0x1C];
} Overlay68ResidentEntry;

/* Both aliases bind to the same overlay-local kind/amount table. */
extern const Overlay68KindPair gOverlay68KindMapInitial[];
extern volatile const s8 gOverlay68KindMapLoop;

/* The runtime relocation stream preserves these five distinct call roles. */
extern Overlay68Probe *overlay68AllocProbeReloc(s32 size, s32 tag);
extern void overlay68FillProbeReloc(s32 kind, Overlay68Probe *probe,
                                    s32 totalBytes, s32 stride);
extern Overlay68ResidentEntry *overlay68GetResidentEntriesReloc(void);
extern s32 overlay68MapResidentIndexReloc(s32 kind);
extern void overlay68FreeProbeReloc(void *probe);

s32 overlay68CheckKind(s32 kind) {
    const Overlay68KindPair *mapping;
    volatile const s8 *loopMapping;
    Overlay68Probe *probe;
    Overlay68ResidentEntry *entries;
    s32 currentKind;
    s32 amount;
    s32 threshold;
    s32 value;
    s32 index;
    s32 result;
    s16 *valueCursor;
    s32 cursorIndex;

    result = 0;
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
        } while (currentKind != -1);
    }

    if (amount != -1) {
        probe = overlay68AllocProbeReloc(sizeof(*probe), 0x85);
        if (probe != NULL) {
            overlay68FillProbeReloc(0x3F, probe,
                                    amount * (s32)sizeof(*probe),
                                    sizeof(*probe));
            entries = overlay68GetResidentEntriesReloc();
            index = overlay68MapResidentIndexReloc(kind);
            threshold = entries[index].thresholdNumerator / 5;
            cursorIndex = 0;
            valueCursor = (s16 *)probe + cursorIndex;
            if (threshold == 0) {
                threshold = 0x7080;
            }

            value = 0x8CA0;
            index = 0;
            if (probe->slots[0] != NULL) {
                do {
                    index++;
                    value = valueCursor[0x10];
                    valueCursor++;
                } while ((index < 4) && (probe->slots[index] != NULL));
            }

            if (threshold < value) {
                result = 1;
            }
        }
        overlay68FreeProbeReloc(probe);
    }

    return result;
}
