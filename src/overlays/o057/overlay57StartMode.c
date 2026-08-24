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
extern s32 gOverlay57State;
extern Overlay57Object gOverlay57Object;
extern u8 gOverlay57ObjectId;
extern void *overlay57StartQueryReloc(s32 value);
extern void overlay57StartPrepareReloc(u8 id);
extern void overlay57StartObjectReloc(u8 id);
extern Overlay57Result *overlay57StartFindResultReloc(u8 id);
extern void overlay57CommitModeReloc(void);
extern void overlay57FinishModeReloc(s32 value);

/* DKR v77/v80 and JFG have no matching donor for this mode transition. */
void overlay57StartMode(s32 value) {
    Overlay57Result *result;
    Overlay57Object *object;

    gOverlay57ModeFlag = 0;
    if (overlay57StartQueryReloc(value) == NULL) {
        object = &gOverlay57Object;
        gOverlay57Timer = 16;
        object->value = 68;
        gOverlay57State = 5;
        overlay57StartPrepareReloc(((u8 *) object)[3]);
        overlay57StartObjectReloc(gOverlay57ObjectId);
        result = overlay57StartFindResultReloc(gOverlay57ObjectId);
        if (result != NULL) {
            result->flags |= 2;
        }
    }
    overlay57CommitModeReloc();
    overlay57FinishModeReloc(value);
}
