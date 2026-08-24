#include "PR/ultratypes.h"

/* DKR v77/v80 object and source checks found no corresponding state setter. */
extern s16 gOverlay40Value0;
extern s16 gOverlay40Value1;
extern s16 gOverlay40Value2;
extern s16 gOverlay40Value3;
extern s16 gOverlay40Value4;
extern s16 gOverlay40Value5;
extern s16 gOverlay40Value6;
extern s16 gOverlay40Value7;

void overlay40SetValues(s32 value0, s32 value1, s32 value2, s32 value3,
                        s32 value4) {
    gOverlay40Value0 = value0;
    gOverlay40Value1 = value1;
    gOverlay40Value4 = value2;
    gOverlay40Value5 = gOverlay40Value3;
    gOverlay40Value6 = value3;
    gOverlay40Value7 = value4;
    gOverlay40Value2 = value4;
}
