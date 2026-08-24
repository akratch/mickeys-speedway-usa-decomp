#include "PR/ultratypes.h"

extern s32 gOverlay57TableMode;
extern u8 gOverlay57TableIndex;
extern s8 gOverlay57TableLabels[5];
extern s8 gOverlay57TableA[][5];
extern s8 gOverlay57TableB[][5];

extern void overlay57ApplyValue(s32 id, s8 slot, s32 value);
extern void overlay57UpdateNode(void);

/*
 * DKR v77/v80 and JFG object scans have no exact donor for this table
 * dispatcher.  Source searches find only generic five-element loops and DKR
 * vehicle-audio tables; none share this mode-selected apply sequence.
 */
void overlay57ApplyTable(void) {
    s32 i;

    if (gOverlay57TableMode != 0) {
        for (i = 0; i != 5; i++) {
            overlay57ApplyValue(0x4C, gOverlay57TableLabels[i],
                                gOverlay57TableB[gOverlay57TableIndex][i]);
        }
    } else {
        for (i = 0; i != 5; i++) {
            overlay57ApplyValue(0x4C, gOverlay57TableLabels[i],
                                gOverlay57TableA[gOverlay57TableIndex][i]);
        }
    }
    overlay57ApplyValue(0x4C, 0, gOverlay57TableIndex);
    overlay57ApplyValue(0x4C, 1, gOverlay57TableIndex);
    overlay57ApplyValue(0x4C, 2, gOverlay57TableIndex);
    overlay57ApplyValue(0x4C, 3, gOverlay57TableIndex);
    overlay57UpdateNode();
}
