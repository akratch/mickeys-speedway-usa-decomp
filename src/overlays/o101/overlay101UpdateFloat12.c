#include "PR/ultratypes.h"

typedef struct Overlay101Float12Output {
    u8 pad00[0xC];
    f32 value;
} Overlay101Float12Output;
typedef struct Overlay101FloatEntry {
    Overlay101Float12Output *output;
    void *owner;
    u8 pad08[8];
    f32 start;
    u8 pad14[0xC];
    f32 end;
    u8 pad24[4];
    s32 elapsed;
    s32 duration;
} Overlay101FloatEntry;

extern s32 gOverlay101EntryCount;

void overlay101UpdateFloat12(Overlay101FloatEntry *entry, s32 step) {
    s32 elapsed;
    f32 value;
    Overlay101Float12Output *output;

    elapsed = (entry->elapsed += step);
    if (elapsed >= entry->duration) {
        entry->owner = NULL;
        value = entry->end;
        gOverlay101EntryCount--;
    } else {
        value = entry->start +
                ((entry->end - entry->start) * (f32)elapsed) /
                    (f32)entry->duration;
    }
    output = entry->output;
    if (output != NULL) {
        output->value = value;
    }
}
