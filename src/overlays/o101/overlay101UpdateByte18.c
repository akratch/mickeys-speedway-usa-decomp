#include "PR/ultratypes.h"

typedef struct Overlay101Byte18Output {
    u8 pad00[0x12];
    u8 value;
} Overlay101Byte18Output;
typedef struct Overlay101Byte18Entry {
    Overlay101Byte18Output *output;
    void *owner;
    s32 start;
    u8 pad0C[0xC];
    s32 end;
    u8 pad1C[0xC];
    s32 elapsed;
    s32 duration;
} Overlay101Byte18Entry;

extern s32 gOverlay101EntryCount;

void overlay101UpdateByte18(Overlay101Byte18Entry *entry, s32 step) {
    s32 elapsed;
    f32 value;
    Overlay101Byte18Output *output;

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
