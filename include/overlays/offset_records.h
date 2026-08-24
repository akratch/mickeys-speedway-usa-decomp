#ifndef OVERLAYS_OFFSET_RECORDS_H
#define OVERLAYS_OFFSET_RECORDS_H

#include "ultra64.h"

typedef struct OverlayOffsetRecord {
    s32 link;
    s32 value;
    s32 metadata;
    s16 x;
    s16 y;
} OverlayOffsetRecord;

#endif
