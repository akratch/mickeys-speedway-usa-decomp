#include "PR/ultratypes.h"

/* Integer time splitter; exact DKR and JFG scans are negative. */
void overlay56SplitTime(s32 value, s32 *minutes, s32 *seconds, s32 *centiseconds) {
    s32 wholeMinutes;
    wholeMinutes = value / 18000;
    *minutes = wholeMinutes;
    *seconds = (value / 300) - (wholeMinutes * 60);
    *centiseconds = (value / 3) % 100;
}
