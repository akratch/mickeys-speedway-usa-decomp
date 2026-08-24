#include "PR/ultratypes.h"

typedef struct Overlay14Object {
    s16 id;
    u8 pad2[10];
    f32 x;
    f32 y;
    f32 z;
} Overlay14Object;

extern s32 gOverlay14ActiveHandle;
extern Overlay14Object *func_800053D0(s32 index);
extern void func_8001EF1C(s16 x, s16 y, s16 z, s16 id);
extern s32 func_800280FC(void);
extern s32 func_800389C0(void);
extern void func_80027F24(s32 handle, s32 first, s32 zero, s32 second,
                         s32 one, s32 zero2);

/* Pinned DKR v77/v80 and JFG scans found no exact finalization donor. */
void overlay14FinalizeActiveHandle(void) {
    Overlay14Object *object;

    if (gOverlay14ActiveHandle != -1) {
        object = func_800053D0(0);
        if (object != 0) {
            func_8001EF1C((s16)(s32)object->x,
                          (s16)(s32)(object->y + 2.0f),
                          (s16)(s32)object->z, object->id);
        }
        func_80027F24(gOverlay14ActiveHandle, func_800280FC(), 0,
                      func_800389C0(), 1, 0);
        gOverlay14ActiveHandle = -1;
    }
}
