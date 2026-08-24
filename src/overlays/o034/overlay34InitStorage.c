#include "PR/ultratypes.h"

typedef struct Overlay34Record {
    u8 bytes[0x68];
} Overlay34Record;

extern Overlay34Record *gOverlay34Records;
extern s32 **gOverlay34Pointers;
extern s32 gOverlay34Count;
extern void *func_8002B280(s32 size, s32 tag);

/* Pinned DKR v77/v80 and JFG scans found no exact donor. */
void overlay34InitStorage(s32 count) {
    s32 *word;
    s32 countdown;
    s32 remaining;
    s32 size;

    size = count * sizeof(Overlay34Record);
    gOverlay34Records = func_8002B280(size, 0x87);
    word = (s32 *)gOverlay34Records;
    remaining = size >> 2;
    countdown = remaining - 1;
    if (remaining != 0) {
        do {
            *word++ = 0;
        } while (countdown--);
    }

    size = count * sizeof(*gOverlay34Pointers);
    gOverlay34Pointers = func_8002B280(size, 0x87);
    word = (s32 *)gOverlay34Pointers;
    remaining = size >> 2;
    countdown = remaining - 1;
    if (remaining != 0) {
        do {
            *word++ = 0;
        } while (countdown--);
    }
    gOverlay34Count = count;
}
