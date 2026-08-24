#include "PR/ultratypes.h"

#define FIELD(base, type, off) (*(type *)((u8 *)(base) + (off)))

typedef struct Vec3f {
    f32 x;
    f32 y;
    f32 z;
} Vec3f;

extern u8 D_EE0[];
extern void func_80029A24(void *, Vec3f *);
extern void func_800150F0(s32, Vec3f *, Vec3f *, f32 *, void *, s32);
extern s32 func_800104B0(Vec3f *, Vec3f *, f32, void *, void *);
extern void func_overlay_029_F00010C4_187E374(void *, s32);
extern void func_overlay_029_F00001C4_187D474(void *);
extern void func_overlay_029_F000023C_187D4EC(void *, void *);

#ifdef NON_MATCHING
void func_overlay_029_F000042C_187D6DC(void *object, void *init) {
    Vec3f position;
    Vec3f offset;
    f32 distance;
    Vec3f *objectPosition;
    void *contact;
    void *source;

    contact = FIELD(object, void *, 0x64);
    position.x = FIELD(object, f32, 0xC);
    position.y = FIELD(object, f32, 0x10);
    offset.x = 0.0f;
    offset.y = 0.0f;
    position.z = FIELD(object, f32, 0x14);
    offset.z = -60.0f;
    source = FIELD(FIELD(init, void * volatile, 0x10), void *, 0x64);

    func_80029A24(FIELD(init, void *, 0x10), &offset);
    objectPosition = (Vec3f *)((u8 *)object + 0xC);
    FIELD(object, f32, 0xC) += offset.x;
    FIELD(object, f32, 0x10) += offset.y;
    FIELD(object, f32, 0x14) += offset.z;

    distance = 8.0f;
    func_800150F0(1, &position, objectPosition, &distance, NULL, 0);
    if (func_800104B0(&position, objectPosition, distance, object, D_EE0) != 0) {
        func_overlay_029_F00010C4_187E374(object, 5);
        return;
    }

    FIELD(FIELD(object, void *, 0x48), u16, 6) |= 2;
    FIELD(FIELD(object, void *, 0x48), void *, 0x70) = FIELD(init, void *, 0x10);
    FIELD(object, s16, 0) = FIELD(init, s16, 0xA);
    FIELD(object, s16, 4) = 0;
    FIELD(object, s32, 0x80) = 1;
    FIELD(object, s16, 2) = FIELD(init, s16, 0xC);
    FIELD(contact, void *, 0) = FIELD(init, void *, 0x10);
    FIELD(contact, s16, 4) = FIELD(source, s16, 0x37C);
    FIELD(contact, u8, 8) = FIELD(source, u8, 0x37E);
    FIELD(contact, u8, 9) = FIELD(source, u8, 0x37F);
    FIELD(contact, u8, 0xA) = FIELD(source, u8, 0x380);
    FIELD(contact, u8, 0x1A) = FIELD(source, u16, 0x1A8) & 8;
    func_overlay_029_F00001C4_187D474(object);
    func_overlay_029_F000023C_187D4EC(object, contact);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o029/overlay29InitializeObject/func_overlay_029_F000042C_187D6DC.s")
#endif
