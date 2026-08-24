#include "PR/ultratypes.h"

typedef struct Overlay101ColorOutput {
    u8 pad00[0xF];
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
} Overlay101ColorOutput;
typedef struct Overlay101ColorEntry {
    Overlay101ColorOutput *output;
    void *owner;
    s16 startRed;
    s16 startGreen;
    s16 startBlue;
    s16 startAlpha;
    u8 pad10[8];
    s16 endRed;
    s16 endGreen;
    s16 endBlue;
    s16 endAlpha;
    u8 pad20[8];
    s32 elapsed;
    s32 duration;
} Overlay101ColorEntry;

extern s32 gOverlay101EntryCount;

void overlay101UpdateColor(Overlay101ColorEntry *entry, s32 step) {
    s32 red;
    s32 green;
    s32 blue;
    s32 alpha;
    s32 elapsed;
    Overlay101ColorOutput *output;

    elapsed = (entry->elapsed += step);
    if (elapsed >= entry->duration) {
        entry->owner = NULL;
        red = entry->endRed;
        green = entry->endGreen;
        blue = entry->endBlue;
        alpha = entry->endAlpha;
        gOverlay101EntryCount--;
    } else {
        red = entry->startRed +
            ((entry->endRed - entry->startRed) * elapsed) / entry->duration;
        green = entry->startGreen +
            ((entry->endGreen - entry->startGreen) * elapsed) / entry->duration;
        blue = entry->startBlue +
            ((entry->endBlue - entry->startBlue) * elapsed) / entry->duration;
        alpha = entry->startAlpha +
            ((entry->endAlpha - entry->startAlpha) * elapsed) / entry->duration;
    }
    output = entry->output;
    if (output != NULL) {
        output->red = red;
        output->green = green;
        output->blue = blue;
        output->alpha = alpha;
    }
}
