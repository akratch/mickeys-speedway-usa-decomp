#include "PR/ultratypes.h"

typedef struct Overlay71SourceCoordinate {
    s16 x;
    s16 y;
    s16 z;
} Overlay71SourceCoordinate;

typedef struct Overlay71CoordinateRecord {
    u8 pad0[6];
    s16 x;
    u8 pad8[2];
    s16 y;
    u8 padC[2];
    s16 z;
} Overlay71CoordinateRecord;

extern s16 gOverlay71Offset;
extern Overlay71CoordinateRecord gOverlay71PrimaryRecords[];
extern Overlay71CoordinateRecord gOverlay71SecondaryRecords[];
extern Overlay71SourceCoordinate gOverlay71PrimaryCoordinates[];
extern Overlay71SourceCoordinate gOverlay71SecondaryCoordinates[];

/* DKR v77/v80 and JFG contain no exact donor for this coordinate updater. */
void overlay71UpdateCoordinates(s32 amount) {
    Overlay71SourceCoordinate *coordinate;
    Overlay71CoordinateRecord *record;
    register s32 previous;
    s16 remaining;
    s16 group;

    gOverlay71Offset = (gOverlay71Offset + (amount * 2)) & 0x3FF;
    group = 0; do {
        if (group == 0) {
            record = gOverlay71PrimaryRecords;
            coordinate = gOverlay71PrimaryCoordinates;
        } else {
            record = gOverlay71SecondaryRecords;
            coordinate = gOverlay71SecondaryCoordinates;
        }
        previous = 7;
        remaining = previous;
        do {
            record->x = coordinate->x + gOverlay71Offset;
            record->y = coordinate->y + gOverlay71Offset;
            record->z = coordinate->z + gOverlay71Offset;
            coordinate++;
            record++;
            previous = remaining;
            remaining--;
        } while (previous);
        group++;
    } while (group < 3);
}
