#ifndef OVERLAYS_OVERLAY_001_H
#define OVERLAYS_OVERLAY_001_H

#include "PR/ultratypes.h"

typedef struct Overlay1Entry {
    u8 bytes[0x1C];
} Overlay1Entry;

typedef struct Overlay1Record {
    u8 bytes[0x94];
} Overlay1Record;

typedef struct Overlay1RingRecord {
    u8 pad00[0x10];
    u16 flags;
    u8 pad12[0x82];
} Overlay1RingRecord;

typedef union Overlay1StartPointer {
    u8 *bytes;
    Overlay1Record *records;
    Overlay1RingRecord *rings;
} Overlay1StartPointer;

extern s32 gOverlay1EntryCount;
extern Overlay1Entry *gOverlay1Entries;
extern Overlay1StartPointer gOverlay1Start;
extern void *gOverlay1TimerState;
extern void *D_1D64;
extern void *D_1D9C;
extern void *D_1DA0;

#endif
