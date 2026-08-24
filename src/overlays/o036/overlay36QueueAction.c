#include "ultra64.h"

typedef struct Overlay36QueueEntry {
    s32 arg0;
    s32 arg1;
    s32 arg2;
    s32 arg3;
    s32 arg4;
} Overlay36QueueEntry;

extern s32 gOverlay36QueueCount;
extern Overlay36QueueEntry *gOverlay36QueueNext;

/* Fixed queue insertion is title-specific; no DKR/JFG donor exists. */
void overlay36QueueAction(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4) {
    if (gOverlay36QueueCount < 16) {
        gOverlay36QueueNext->arg0 = arg0;
        gOverlay36QueueNext->arg1 = arg1;
        gOverlay36QueueNext->arg2 = arg2;
        gOverlay36QueueNext->arg3 = arg3;
        gOverlay36QueueNext->arg4 = arg4;
        gOverlay36QueueNext++;
        gOverlay36QueueCount++;
    }
}
