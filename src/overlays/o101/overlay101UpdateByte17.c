#include "PR/ultratypes.h"

typedef struct Overlay101ByteOutput17 { u8 pad00[0x17]; u8 value; } Overlay101ByteOutput17;
typedef struct Overlay101ByteEntry17 {
    Overlay101ByteOutput17 *output; void *owner; s32 start;
    u8 pad0C[0xC]; s32 end; u8 pad1C[0xC]; s32 elapsed; s32 duration;
} Overlay101ByteEntry17;
extern s32 gOverlay101EntryCount;

void overlay101UpdateByte17(Overlay101ByteEntry17 *entry, s32 step) {
    s32 value;
    s32 elapsed;
    Overlay101ByteOutput17 *output;

    elapsed = (entry->elapsed += step);
    if (elapsed >= entry->duration) {
        entry->owner = NULL;
        value = entry->end;
        gOverlay101EntryCount--;
    } else {
        value = entry->start +
            ((entry->end - entry->start) * elapsed) / entry->duration;
    }
    output = entry->output;
    if (output != NULL) output->value = value;
}
