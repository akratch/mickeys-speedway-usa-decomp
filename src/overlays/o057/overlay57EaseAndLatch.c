#include "PR/ultratypes.h"

typedef struct O57Ease28B4Point {
    u8 pad00[0x0C];
    s16 x;
    s16 y;
} O57Ease28B4Point;

typedef struct O57Ease28B4Transform {
    s16 angleX;
    s16 angleY;
    s16 angleZ;
    u8 pad06[6];
    f32 x;
    f32 y;
    f32 z;
} O57Ease28B4Transform;

typedef struct O57Ease28B4Primary {
    u8 pad00[8];
    O57Ease28B4Transform *source;
} O57Ease28B4Primary;

typedef struct O57Ease28B4Secondary {
    u8 pad00[0x16];
    u8 flags;
    u8 pad17[9];
    O57Ease28B4Transform *target;
} O57Ease28B4Secondary;

extern O57Ease28B4Point gO57Ease28B4Record21C;
extern O57Ease28B4Point gO57Ease28B4Record23C;
extern s32 gO57Ease28B4State144;
extern s32 gO57Ease28B4State118;
extern s32 gO57Ease28B4State180;
extern u8 gO57Ease28B4Id183;
extern s32 gO57Ease28B4Latch50C;
extern u32 gO57Ease28B4InputFlagsReloc;

extern O57Ease28B4Primary *o57Ease28B4LookupPrimaryReloc(u8 id);
extern void o57Ease28B4CommandReloc(s32 command, s32 argument);
extern O57Ease28B4Primary *o57Ease28B4LookupAgainReloc(u8 id);
extern O57Ease28B4Secondary *o57Ease28B4LookupSecondaryReloc(s32 id);
extern void o57Ease28B4NotifyFirstReloc(s32 id);
extern void o57Ease28B4NotifySecondReloc(s32 id);
extern void o57Ease28B4AlternateCommandReloc(s32 command, s32 argument);
extern void o57Ease28B4AlternateFinishReloc(s32 argument);

/* Overlay 57 text +0x28B4..+0x2C28. */
#ifdef NON_MATCHING
void overlay57EaseAndLatch(s32 updateRate) {
    O57Ease28B4Primary *primary;
    O57Ease28B4Secondary *secondary;
    O57Ease28B4Transform *target;
    u32 flags;
    s32 i;
    s32 x;
    s32 y;

    gO57Ease28B4State144 = 0;

    i = 0;
    if (updateRate > 0) {
        do {
            x = gO57Ease28B4Record21C.x;
            y = gO57Ease28B4Record21C.y;
            i++;
            gO57Ease28B4Record21C.y = (s16)(y + ((0xBE - y) >> 3));
            gO57Ease28B4Record21C.x = (s16)(x + ((0x104 - x) >> 3));
        } while (i != updateRate);
    }

    i = 0;
    if (updateRate > 0) {
        do {
            x = gO57Ease28B4Record23C.x;
            y = gO57Ease28B4Record23C.y;
            i++;
            gO57Ease28B4Record23C.y = (s16)(y + ((0xBE - y) >> 3));
            gO57Ease28B4Record23C.x = (s16)(x + ((0x32 - x) >> 3));
        } while (i != updateRate);
    }

    flags = gO57Ease28B4InputFlagsReloc;
    if (((flags & 0x9000) != 0) && (gO57Ease28B4Latch50C == 0)) {
        primary = o57Ease28B4LookupPrimaryReloc(gO57Ease28B4Id183);
        if (primary == NULL) {
            return;
        }
        o57Ease28B4CommandReloc(0xC, 0);
        (void)o57Ease28B4LookupAgainReloc(gO57Ease28B4Id183);
        secondary = o57Ease28B4LookupSecondaryReloc(0x29);
        if (secondary != NULL) {
            target = secondary->target;
            target->x = primary->source->x;
            target->y = primary->source->y;
            target->z = primary->source->z;
            target->angleX = primary->source->angleX;
            target->angleY = primary->source->angleY;
            target->angleZ = primary->source->angleZ;
            o57Ease28B4NotifyFirstReloc(0x29);
            o57Ease28B4NotifySecondReloc(0x29);
            gO57Ease28B4State180 = 0x29;
            secondary->flags |= 2;
        }
        gO57Ease28B4Latch50C = 1;
        gO57Ease28B4State118 = 14;
        return;
    }

    if (((flags & 0x4000) != 0) && (gO57Ease28B4Latch50C == 0)) {
        o57Ease28B4AlternateCommandReloc(0xD, 0);
        o57Ease28B4AlternateFinishReloc(1);
        gO57Ease28B4Latch50C = 1;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o057/overlay57EaseAndLatch/func_overlay_057_F00028B4_18A64AC.s")
#endif
