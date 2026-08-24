#include "PR/ultratypes.h"

typedef struct Overlay101Output8 { u8 pad00[8]; s16 x; s16 y; } Overlay101Output8;
typedef struct Overlay101TimedEntry8 {
    Overlay101Output8 *output; void *owner; s32 startX; s32 startY;
    u8 pad10[8]; s32 endX; s32 endY; u8 pad20[8];
    s32 elapsed; s32 duration;
} Overlay101TimedEntry8;
extern s32 gOverlay101EntryCount;

void overlay101UpdateEntry8(Overlay101TimedEntry8 *entry, s32 step) {
    s32 x;
    s32 y;
    s32 elapsed;
    Overlay101Output8 *output;

    elapsed = (entry->elapsed += step);
    if (elapsed >= entry->duration) {
        entry->owner = NULL;
        x = entry->endX;
        y = entry->endY;
        gOverlay101EntryCount--;
    } else {
        x = entry->startX +
            ((entry->endX - entry->startX) * elapsed) / entry->duration;
        y = entry->startY +
            ((entry->endY - entry->startY) * elapsed) / entry->duration;
    }
    output = entry->output;
    if (output != NULL) {
        output->x = x;
        output->y = y;
    }
}
