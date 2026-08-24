#include "PR/ultratypes.h"

typedef struct O57Tail2C28Smooth {
    u8 pad00[0xC];
    s16 x;
    s16 y;
} O57Tail2C28Smooth;

typedef struct O57Tail2C28Position {
    u8 pad00[0xC];
    f32 x;
    f32 y;
    f32 z;
} O57Tail2C28Position;

extern O57Tail2C28Smooth gO57Tail2C28Record21C;
extern O57Tail2C28Smooth gO57Tail2C28Record23C;
extern O57Tail2C28Position *gO57Tail2C28Reference14C;
extern f32 gO57Tail2C28DistanceLimit104;
extern s32 gO57Tail2C28QueryWord180;
extern s32 gO57Tail2C28Timer118;
extern s32 gO57Tail2C28Completion50C;

extern void o57Tail2C28ResetReloc(s32 value);
extern O57Tail2C28Position *o57Tail2C28GetPositionReloc(void);
extern void o57Tail2C28EmitReloc(s32 type, u32 scaleBits, u32 valueBits,
                                 s32 arg3, s32 arg4, s32 arg5, s32 arg6);
extern void *o57Tail2C28QueryReloc(s32 value);
extern void o57Tail2C28StartReloc(s32 type, s32 arg1, s32 arg2, s32 mode,
                                  s32 arg4, s32 arg5);
extern void o57Tail2C28FinishReloc(void);

/* Overlay 57 text +0x2C28..+0x2F48. */
void overlay57SmoothAndCheckDistance(s32 smoothingSteps) {
    s32 i;
    s32 x;
    s32 y;
    f32 dx;
    f32 dy;
    f32 dz;
    O57Tail2C28Position *position;
    O57Tail2C28Position *reference;

    i = 0;
    if (smoothingSteps > 0) {
        do {
            x = gO57Tail2C28Record21C.x;
            y = gO57Tail2C28Record21C.y;
            i++;
            gO57Tail2C28Record21C.y =
                (s16)(y + ((0xBE - y) >> 3));
            gO57Tail2C28Record21C.x =
                (s16)(x + ((0x17C - x) >> 3));
        } while (i != smoothingSteps);
    }

    i = 0;
    if (smoothingSteps > 0) {
        do {
            x = gO57Tail2C28Record23C.x;
            y = gO57Tail2C28Record23C.y;
            i++;
            gO57Tail2C28Record23C.y =
                (s16)(y + ((0xBE - y) >> 3));
            gO57Tail2C28Record23C.x =
                (s16)(x + ((-0x46 - x) >> 3));
        } while (i != smoothingSteps);
    }

    o57Tail2C28ResetReloc(0);
    position = o57Tail2C28GetPositionReloc();
    reference = gO57Tail2C28Reference14C;
    if (reference != NULL) {
        dx = reference->x - position->x;
        dy = reference->y - position->y;
        dz = reference->z - position->z;
        if (((dx * dx) + (dy * dy)) + (dz * dz) <
            gO57Tail2C28DistanceLimit104) {
            o57Tail2C28EmitReloc(4, 0x3EAE147B, 0xBF800000,
                                 0, 0, 0, 0);
        }
    }

    if (o57Tail2C28QueryReloc(gO57Tail2C28QueryWord180) != NULL) {
        gO57Tail2C28Timer118 = 0x11;
        o57Tail2C28StartReloc(0x1D, 0, 0, 9, 1, 0);
        gO57Tail2C28Completion50C = 1;
        o57Tail2C28FinishReloc();
    }
}
