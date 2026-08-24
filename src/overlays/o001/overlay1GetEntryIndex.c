#include "PR/ultratypes.h"

typedef struct Overlay1Entry {
    u8 bytes[0x1C];
} Overlay1Entry;

extern Overlay1Entry *gOverlay1Entries;

s32 overlay1GetEntryIndex(Overlay1Entry *entry) {
    return entry - gOverlay1Entries;
}
