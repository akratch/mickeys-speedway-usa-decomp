#include "PR/ultratypes.h"

typedef struct Overlay57Object {
    s32 value;
} Overlay57Object;

typedef struct Overlay57Result {
    u8 pad0[0x16];
    u8 flags;
} Overlay57Result;

extern s32 gOverlay57ModeFlag;
extern s32 gOverlay57Timer;
extern Overlay57Object gOverlay57Object;
extern u8 gOverlay57ObjectId;
extern void *overlay57QueryReloc(s32 value);
extern void overlay57PrepareReloc(u8 id);
extern void overlay57StartReloc(u8 id);
extern Overlay57Result *overlay57FindResultReloc(u8 id);
extern void overlay57FinishReloc(s32 value);

/* DKR v77/v80 and JFG checks found no matching mode-initialization donor. */
void overlay57InitializeMode(s32 value) {
    Overlay57Result *result;
    Overlay57Object *object;

    gOverlay57ModeFlag = 1;
    if (overlay57QueryReloc(value) == NULL) {
        object = &gOverlay57Object;
        gOverlay57Timer = 20;
        object->value = 84;
        overlay57PrepareReloc(((u8 *) object)[3]);
        overlay57StartReloc(gOverlay57ObjectId);
        result = overlay57FindResultReloc(gOverlay57ObjectId);
        if (result != NULL) {
            result->flags |= 2;
        }
    }
    overlay57FinishReloc(value);
}
