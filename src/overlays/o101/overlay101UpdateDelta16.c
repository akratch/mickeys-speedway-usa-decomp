#include "PR/ultratypes.h"

typedef struct Overlay101Delta16Output {
    u8 pad00[0x10];
    s16 value;
} Overlay101Delta16Output;
typedef struct Overlay101DeltaEntry {
    Overlay101Delta16Output *output;
    void *owner;
    s32 start;
    u8 pad0C[0xC];
    s32 delta;
    u8 pad1C[0xC];
    s32 elapsed;
    s32 duration;
} Overlay101DeltaEntry;

extern s32 gOverlay101EntryCount;

void overlay101UpdateDelta16(Overlay101DeltaEntry *entry, s32 step) {
    s32 elapsed;
    s32 value;
    Overlay101Delta16Output *output;

    elapsed = (entry->elapsed += step);
    value = entry->start;
    if (elapsed >= entry->duration) {
        entry->owner = NULL;
        value += entry->delta;
        gOverlay101EntryCount--;
    } else {
        value += (entry->delta * elapsed) / entry->duration;
    }
    output = entry->output;
    if (output != NULL) {
        output->value = value;
    }
}
