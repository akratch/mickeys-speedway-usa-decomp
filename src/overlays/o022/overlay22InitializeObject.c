#include "PR/ultratypes.h"

#define FIELD(base, type, off) (*(type *)((u8 *)(base) + (off)))

typedef struct Vec3f {
    f32 x;
    f32 y;
    f32 z;
} Vec3f;

extern f32 D_EE0;
extern u8 D_A7C[];
extern void *D_0[];
extern s32 D_30;
extern void func_80029A24(void *, Vec3f *);
extern void func_800150F0(s32, Vec3f *, Vec3f *, f32 *, void *, s32);
extern s32 func_800104B0(Vec3f *, Vec3f *, f32, void *, void *);
extern void func_8003E99C(void *, s32);
extern void func_80006A50(void *);
extern s16 func_8000F690(f32, f32, f32);
extern void func_overlay_022_F0000D30_1878E38(void *, s32, s32 *);

#ifdef NON_MATCHING
void func_overlay_022_F0000000_1878108(void *object, void *init) {
    Vec3f position;
    Vec3f offset;
    f32 distance;
    s32 keep;
    Vec3f *objectPosition;
    void * volatile planes;
    void *planeArg;
    void *temp_a0;
    void *contact;

    contact = FIELD(object, void *, 0x64);
    FIELD(contact, f32, 4) = (f32)FIELD(init, s32, 0xC);
    FIELD(contact, s8, 0) = (s8)FIELD(init, s16, 0xA);
    FIELD(contact, void *, 0x38) = FIELD(init, void *, 0x14);
    FIELD(contact, s16, 2) = 1;
    position.x = FIELD(object, f32, 0xC);
    position.y = FIELD(object, f32, 0x10);
    offset.x = 0.0f;
    offset.y = 0.0f;
    position.z = FIELD(object, f32, 0x14);
    offset.z = FIELD(init, f32, 0x10);
    func_80029A24(FIELD(contact, void *, 0x38), &offset);
    objectPosition = (Vec3f *)((u8 *)object + 0xC);
    FIELD(object, f32, 0xC) += offset.x;
    FIELD(object, f32, 0x10) += offset.y;
    FIELD(object, f32, 0x14) += offset.z;
    keep = 1;
    distance = D_EE0;
    func_800150F0(1, &position, objectPosition, &distance, 0, 1);
    planeArg = D_A7C;
    planes = planeArg;
    if ((func_800104B0(&position, objectPosition, distance, object, planeArg) != 0) &&
        (FIELD(contact, u8, 1) & 4)) {
        FIELD(object, s32, 0x80) |= 2;
        func_8003E99C(object, 1);
        func_80006A50(object);
        keep = 0;
    }
    if (keep != 0) {
        if (FIELD(contact, s8, 0) == 2) {
            if (!(FIELD(contact, u8, 1) & 2)) {
                FIELD(contact, u8, 1) = 0;
                position.x = FIELD(object, f32, 0xC);
                position.y = FIELD(object, f32, 0x10) - 5.0f;
                position.z = FIELD(object, f32, 0x14);
                if (func_800104B0(objectPosition, &position, distance, object, planes) == 0) {
                    FIELD(contact, s8, 0) = 1;
                    FIELD(object, s16, 4) = 0x4000;
                }
            }
        } else {
            FIELD(object, f32, 0x24) = -FIELD(contact, f32, 4);
            func_80029A24(FIELD(contact, void *, 0x38),
                          (Vec3f *)((u8 *)object + 0x1C));
            FIELD(object, s16, 4) = 0x4000;
        }
        FIELD(object, s16, 0x2E) = func_8000F690(
            FIELD(object, f32, 0xC), FIELD(object, f32, 0x10),
            FIELD(object, f32, 0x14));
        temp_a0 = FIELD(object, void *, 0x48);
        FIELD(temp_a0, void *, 0x70) = FIELD(contact, void *, 0x38);
        temp_a0 = FIELD(object, void *, 0x48);
        FIELD(temp_a0, u16, 6) |= 2;
        FIELD(contact, f32, 0x2C) = FIELD(object, f32, 0xC);
        FIELD(contact, f32, 0x30) = FIELD(object, f32, 0x10);
        FIELD(contact, s16, 2) = 0;
        FIELD(contact, f32, 0x34) = FIELD(object, f32, 0x14);
        if (D_30 < 12) {
            D_0[D_30++] = object;
            return;
        }
        func_overlay_022_F0000D30_1878E38(D_0[0], 5, &D_30);
        D_0[D_30++] = object;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o022/overlay22InitializeObject/func_overlay_022_F0000000_1878108.s")
#endif
