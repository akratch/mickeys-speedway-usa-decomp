#include "PR/ultratypes.h"

/* DKR v77/v80 has related menu projection/rendering, but no exact helper. */
typedef struct Overlay40Entry {
    s8 state;
    s8 scales[3];
    u8 red;
    u8 green;
    u8 blue;
    u8 id;
} Overlay40Entry;

typedef struct Overlay40Position {
    u8 pad00[0xC];
    f32 x;
    f32 y;
    f32 z;
} Overlay40Position;

typedef struct Overlay40Object {
    u8 pad00[8];
    Overlay40Position *position;
} Overlay40Object;

extern Overlay40Entry gOverlay40Entries[8];
extern Overlay40Object **gOverlay40Objects;
extern s32 overlay40ProjectReloc(f32 x, f32 y, f32 z, f32 *screenX,
                                 f32 *screenY, s32 flags);
extern void overlay40DrawFrame(void *displayList, s32 x, s32 y, s32 width,
                               s32 height, s32 red, s32 green, s32 blue,
                               s32 alpha);

void overlay40DrawEntries(void *displayList) {
    Overlay40Entry *entry;
    Overlay40Object *object;
    Overlay40Position *position;
    s8 *scaleValue;
    s32 remaining;
    f32 screenX;
    f32 screenY;
    s32 scaleRemaining;
    s32 alpha;
    s32 scale;
    s32 lastScale;

    entry = gOverlay40Entries;
    /* IDO schedules both relocated table bases before the counter on this line. */
    remaining = 7; do {
        if (entry->state != -1) {
            object = gOverlay40Objects[entry->id];
            if (object != NULL) {
                position = object->position;
                if (position != NULL &&
                    overlay40ProjectReloc(position->x, position->y, position->z,
                                          &screenX, &screenY, 1) != 0) {
                    scaleValue = entry->scales;
                    alpha = 0xA0;
                    lastScale = -1;
                    scaleRemaining = 2;
                    do {
                        scale = (*scaleValue * 8) + 0x10;
                        if (scale != lastScale) {
                            overlay40DrawFrame(displayList,
                                               (s32) screenX - scale,
                                               (s32) screenY - scale,
                                               scale * 2, scale * 2,
                                               entry->red, entry->green,
                                               entry->blue, alpha);
                            lastScale = scale;
                        }
                        alpha -= 0x34;
                        scaleValue++;
                    } while (scaleRemaining--);
                }
            }
        }
        entry++;
    } while (remaining--);
}
