#include "PR/ultratypes.h"

typedef struct Vec3f {
    f32 x;
    f32 y;
    f32 z;
} Vec3f;

typedef struct Overlay29SourceState {
    u8 pad000[0x1A8];
    u16 flags;
    u8 pad1AA[0x1D2];
    s16 recordIndex;
    u8 selector0;
    u8 selector1;
    u8 selector2;
} Overlay29SourceState;

typedef struct Overlay29Entity {
    u8 pad00[6];
    u16 flags;
    u8 pad08[0x68];
    void *owner;
} Overlay29Entity;

typedef struct Overlay29Contact {
    void *owner;
    s16 recordIndex;
    u8 pad06[2];
    u8 selector0;
    u8 selector1;
    u8 selector2;
    u8 pad0B[0xF];
    u8 flags;
} Overlay29Contact;

typedef struct Overlay29Object {
    s16 angle0;
    s16 angle1;
    s16 angle2;
    u8 pad06[6];
    Vec3f position;
    u8 pad18[0x30];
    Overlay29Entity *entity;
    u8 pad4C[0x18];
    Overlay29Contact *contact;
    u8 pad68[0x18];
    s32 active;
} Overlay29Object;

typedef struct Overlay29Init {
    u8 pad00[0xA];
    s16 angle0;
    s16 angle1;
    u8 pad0E[2];
    Overlay29Object * volatile object;
} Overlay29Init;

extern u8 D_EE0[];
extern void func_80029A24(void *, Vec3f *);
extern void func_800150F0(s32, Vec3f *, Vec3f *, f32 *, void *, s32);
extern s32 func_800104B0(Vec3f *, Vec3f *, f32, void *, void *);
extern void func_overlay_029_F00010C4_187E374(void *, s32);
extern void func_overlay_029_F00001C4_187D474(void *);
extern void func_overlay_029_F000023C_187D4EC(void *, void *);

/*
 * NON_MATCHING: typed object/state access and direct reuse of object->position
 * reproduce the exact 0x198-byte extent and 0x58-byte frame, improving the
 * baseline from 37 to 15 differing instruction words. The first mismatch is
 * +0x14; the remaining residual is the early volatile source-load/local-init
 * schedule. The full 119-combination flag lattice produced no exact variant.
 */
#ifdef NON_MATCHING
void func_overlay_029_F000042C_187D6DC(Overlay29Object *object,
                                       Overlay29Init *init) {
    Overlay29Contact *contact;
    Overlay29SourceState *source;
    Vec3f position;
    Vec3f offset;
    f32 distance;

    contact = object->contact;
    position.x = object->position.x;
    position.y = object->position.y;
    offset.x = 0.0f;
    offset.y = 0.0f;
    position.z = object->position.z;
    offset.z = -60.0f;
    source = (Overlay29SourceState *)init->object->contact;

    func_80029A24(init->object, &offset);
    object->position.x += offset.x;
    object->position.y += offset.y;
    object->position.z += offset.z;

    distance = 8.0f;
    func_800150F0(1, &position, &object->position, &distance, NULL, 0);
    if (func_800104B0(&position, &object->position, distance, object, D_EE0) != 0) {
        func_overlay_029_F00010C4_187E374(object, 5);
        return;
    }

    object->entity->flags |= 2;
    object->entity->owner = init->object;
    object->angle0 = init->angle0;
    object->angle1 = init->angle1;
    object->angle2 = 0;
    object->active = 1;
    contact->owner = init->object;
    contact->recordIndex = source->recordIndex;
    contact->selector0 = source->selector0;
    contact->selector1 = source->selector1;
    contact->selector2 = source->selector2;
    contact->flags = source->flags & 8;
    func_overlay_029_F00001C4_187D474(object);
    func_overlay_029_F000023C_187D4EC(object, contact);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o029/overlay29InitializeObject/func_overlay_029_F000042C_187D6DC.s")
#endif
