#include "PR/ultratypes.h"

/* DKR v77/v80 and JFG have no exact donor for this frame-step update. */

typedef struct Overlay101FrameOutput {
    u8 pad00[0xC];
    u8 frameCount;
    u8 frame;
} Overlay101FrameOutput;

typedef struct Overlay101FrameEntry {
    Overlay101FrameOutput *output;
    s32 owner;
    s32 delay;
    u8 pad0C[0x1C];
    s32 elapsed;
    s32 duration;
} Overlay101FrameEntry;

extern s32 gOverlay101EntryCount;

void overlay101UpdateFrames(Overlay101FrameEntry *entry, s32 step) {
    Overlay101FrameOutput *output;
    s32 delay;

    delay = (entry->delay -= step);
    if (delay < 0) {
        output = entry->output;
        if (output != NULL) {
            entry->delay = 0;
            entry->elapsed -= delay;
            if (entry->elapsed >= entry->duration) {
                do {
                    output->frame++;
                    entry->elapsed -= entry->duration;
                } while (entry->elapsed >= entry->duration);
            }
            if (output->frame >= output->frameCount) {
                output->frame = output->frameCount;
                entry->owner = 0;
                gOverlay101EntryCount--;
            }
        } else {
            entry->owner = 0;
            gOverlay101EntryCount--;
        }
    }
}
