#include "PR/ultratypes.h"

typedef struct Overlay101DispatchEntry {
    u8 pad00[4];
    s32 type;
    u8 pad08[0x28];
} Overlay101DispatchEntry;

extern s32 gOverlay101EntryCount;
extern Overlay101DispatchEntry gOverlay101Entries[40];

extern void overlay101UpdateEntry(Overlay101DispatchEntry *entry, void *value);
extern void overlay101UpdateEntry12(Overlay101DispatchEntry *entry, void *value);
extern void overlay101UpdateByte17(Overlay101DispatchEntry *entry, void *value);
extern void overlay101UpdateByte16(Overlay101DispatchEntry *entry, void *value);
extern void overlay101UpdateEntry8(Overlay101DispatchEntry *entry, void *value);
extern void overlay101UpdateEntry8B(Overlay101DispatchEntry *entry, void *value);
extern void overlay101UpdateFloat12(Overlay101DispatchEntry *entry, void *value);
extern void overlay101UpdateDelta16(Overlay101DispatchEntry *entry, void *value);
extern void overlay101UpdateByte18(Overlay101DispatchEntry *entry, void *value);
extern void overlay101UpdateEntry8C(Overlay101DispatchEntry *entry, void *value);
extern void overlay101UpdateColor(Overlay101DispatchEntry *entry, void *value);
extern void overlay101UpdateFrames(Overlay101DispatchEntry *entry, void *value);
extern void overlay101UpdateGlobalPair(Overlay101DispatchEntry *entry, void *value);

/* Pinned DKR v77/v80 and JFG scans classify overlay 101 as no donor. */
#ifdef NON_MATCHING
void overlay101DispatchActive(void *value) {
    Overlay101DispatchEntry *entry;
    s32 remaining;

    if (gOverlay101EntryCount > 0) {
        entry = gOverlay101Entries;
        remaining = 39;
        do {
            switch (entry->type) {
                case 1:
                    overlay101UpdateEntry(entry, value);
                    break;
                case 2:
                    overlay101UpdateEntry12(entry, value);
                    break;
                case 3:
                    overlay101UpdateByte17(entry, value);
                    break;
                case 4:
                    overlay101UpdateByte16(entry, value);
                    break;
                case 5:
                    overlay101UpdateEntry8(entry, value);
                    break;
                case 6:
                    overlay101UpdateEntry8B(entry, value);
                    break;
                case 7:
                    overlay101UpdateFloat12(entry, value);
                    break;
                case 8:
                    overlay101UpdateDelta16(entry, value);
                    break;
                case 9:
                    overlay101UpdateByte18(entry, value);
                    break;
                case 10:
                    overlay101UpdateEntry8C(entry, value);
                    break;
                case 11:
                    overlay101UpdateColor(entry, value);
                    break;
                case 12:
                    overlay101UpdateFrames(entry, value);
                    break;
                case 13:
                    overlay101UpdateGlobalPair(entry, value);
                    break;
                case 14:
                case 15:
                case 16:
                    break;
            }
            entry++;
        } while (remaining--);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o101/overlay101DispatchActive/func_overlay_101_F0001A38_18DD258.s")
#endif
