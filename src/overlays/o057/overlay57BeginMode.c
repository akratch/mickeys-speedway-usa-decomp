#include "PR/ultratypes.h"

typedef struct Overlay57Object {
    s32 value;
} Overlay57Object;

typedef struct Overlay57Result {
    u8 pad0[0x16];
    u8 flags;
} Overlay57Result;

typedef struct Overlay57ModeValue {
    s32 value;
} Overlay57ModeValue;

extern s32 gOverlay57ModeFlag;
extern s32 gOverlay57Timer;
extern s32 gOverlay57Delay;
extern Overlay57Object gOverlay57Object;
extern u8 gOverlay57ObjectId;
extern void *overlay57BeginQueryReloc(s32 value);
extern void overlay57BeginPrepareReloc(u8 id);
extern void overlay57BeginStartReloc(u8 id);
extern Overlay57Result *overlay57BeginFindResultReloc(u8 id);
extern void overlay57BeginFinishReloc(s32 value);

/* DKR v77/v80 and JFG have no matching donor; 0xA00 is generic-only. */
void overlay57BeginMode(Overlay57ModeValue value) {
    Overlay57Result *result;
    Overlay57Object *object;

    gOverlay57ModeFlag = 0;
    if (overlay57BeginQueryReloc(value.value) == NULL) {
        gOverlay57Delay = 0xA00;
        object = &gOverlay57Object;
        gOverlay57Timer = 13;
        object->value = 0x3F;
        overlay57BeginPrepareReloc(((u8 *) object)[3]);
        overlay57BeginStartReloc(gOverlay57ObjectId);
        result = overlay57BeginFindResultReloc(gOverlay57ObjectId);
        if (result != NULL) {
            result->flags |= 2;
        }
        value.value = 1;
        overlay57BeginFinishReloc(value.value);
    }
}
