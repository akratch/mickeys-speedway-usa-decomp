#include "PR/ultratypes.h"

typedef struct Overlay27CoordinateRecord {
    u8 pad0;
    u8 firstIndex;
    u8 secondIndex;
    u8 thirdIndex;
    s16 firstX;
    s16 firstY;
    s16 secondX;
    s16 secondY;
    s16 thirdX;
    s16 thirdY;
} Overlay27CoordinateRecord;

extern Overlay27CoordinateRecord gOverlay27CoordinateRecords[];
extern s16 gOverlay27XCoordinates[];
extern s16 gOverlay27YCoordinates[];
extern s32 gOverlay27XOffset;
extern s32 gOverlay27YOffset;

/* DKR v77/v80 and JFG contain no exact donor for this table transform. */
void overlay27UpdateCoordinates(s32 amount) {
    Overlay27CoordinateRecord *record;
    s32 xOffset;
    s32 remaining;

    xOffset = gOverlay27XOffset =
        (gOverlay27XOffset + (-amount * 12)) & 0x3FF;
    amount = gOverlay27YOffset =
        (gOverlay27YOffset + (amount * 48)) & 0x3FF;

    record = gOverlay27CoordinateRecords;
    remaining = 9;
    do {
        record->firstX = gOverlay27XCoordinates[record->firstIndex] +
                         xOffset;
        record->firstY = gOverlay27YCoordinates[record->firstIndex] +
                         amount;
        record->secondX = gOverlay27XCoordinates[record->secondIndex] +
                          xOffset;
        record->secondY = gOverlay27YCoordinates[record->secondIndex] +
                          amount;
        record->thirdX = gOverlay27XCoordinates[record->thirdIndex] +
                         xOffset;
        record->thirdY = gOverlay27YCoordinates[record->thirdIndex] +
                         amount;
        record++;
    } while (remaining--);
}
