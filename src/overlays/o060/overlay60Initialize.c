#include "PR/ultratypes.h"

typedef struct Overlay60SpawnDesc {
    s16 objectId;
    s8 size;
    u8 pad03;
    s16 x;
    s16 y;
    s16 z;
    u8 pad0A;
    u8 alpha;
    u8 pad0C;
} Overlay60SpawnDesc;

typedef struct Overlay60Inner {
    u8 pad00[8];
    s16 mode;
} Overlay60Inner;

typedef struct Overlay60Object {
    u8 pad00[0xC];
    f32 x;
    f32 y;
    u8 pad14[2];
    u8 flags;
    u8 pad17[0x51];
    Overlay60Inner **inner;
} Overlay60Object;

extern u8 *gOverlay60SourceReloc;
extern u8 gOverlay60PostSourceReloc[];
extern u32 gOverlay60Data00[10];
extern u32 gOverlay60Data28[4];
extern u32 gOverlay60Data38[2];
extern s32 gOverlay60Data40;
extern s32 gOverlay60Data44;
extern s32 gOverlay60Data48;
extern s32 gOverlay60Data4C;
extern s32 gOverlay60Data58;

extern u8 gOverlay60Bss00[];
extern u32 gOverlay60Bss20;
extern u32 gOverlay60Bss40;
extern u32 gOverlay60Bss60;
extern u32 gOverlay60Bss80;
extern Overlay60Object *gOverlay60ObjectC8;
extern Overlay60Object *gOverlay60ObjectCC;
extern Overlay60Object *gOverlay60ObjectCCY;
extern Overlay60Object *gOverlay60ObjectD0;
extern Overlay60Object *gOverlay60ObjectD0Y;
extern Overlay60Object *gOverlay60ObjectD4;
extern Overlay60Object *gOverlay60ObjectD4Y;
extern s16 gOverlay60CoordsD8[];
extern s16 gOverlay60CoordE8;
extern s16 gOverlay60CoordEA;
extern s16 gOverlay60CoordEC;
extern s16 gOverlay60CoordEE;
extern s16 gOverlay60CoordF0;
extern s16 gOverlay60CoordF2;
extern s16 gOverlay60CoordF4;
extern s16 gOverlay60CoordF6;
extern s32 gOverlay60Bss174;
extern u32 gOverlay60Bss144;
extern u32 gOverlay60Bss1E4;
extern u32 gOverlay60Bss1EC;
extern u32 gOverlay60Bss0C;

extern Overlay60Object *overlay60SpawnReloc(Overlay60SpawnDesc *desc, s32 arg1);
extern void overlay60ConfigureReloc(Overlay60Object *object, s32 arg1, s32 arg2,
                                    f32 arg3);
extern void overlay60InitReloc(void *arg0);
extern s32 overlay60ReadyReloc(void);
extern void overlay60SelectReloc(u8 index);
extern Overlay60Object *overlay60FindReloc(u8 index);

#define SOURCE(type, offset) (*(type *)(gOverlay60SourceReloc + (offset)))

#ifdef NON_MATCHING
void func_overlay_060_F0000000_18B9DD8(void) {
    Overlay60Object **objects;
    Overlay60SpawnDesc desc;
    Overlay60Object *object;
    s16 *coordinate;
    s32 i;

    gOverlay60Data00[0] = SOURCE(u32, 0x44);
    gOverlay60Data00[1] = SOURCE(u32, 0x1C8);
    gOverlay60Data00[2] = SOURCE(u32, 0x1E4);
    gOverlay60Data00[3] = SOURCE(u32, 0x48);
    gOverlay60Data00[4] = SOURCE(u32, 0x40);
    gOverlay60Data00[5] = SOURCE(u32, 0x3C);
    gOverlay60Data00[6] = SOURCE(u32, 0x4C);
    gOverlay60Data00[7] = SOURCE(u32, 0x278);
    gOverlay60Data00[8] = SOURCE(u32, 0x1CC);
    gOverlay60Data00[9] = SOURCE(u32, 0x1D0);

    gOverlay60Data28[0] = SOURCE(u32, 0x00);
    gOverlay60Data28[1] = SOURCE(u32, 0x04);
    gOverlay60Data28[2] = SOURCE(u32, 0x08);
    gOverlay60Data28[3] = SOURCE(u32, 0x0C);
    gOverlay60Data38[0] = SOURCE(u32, 0x1C);
    gOverlay60Data38[1] = SOURCE(u32, 0x18);

    gOverlay60Data40 = 0;
    gOverlay60Data44 = 0;
    gOverlay60Data48 = 0x9B;
    gOverlay60Data4C = -1;

    i = 0;
    objects = &gOverlay60ObjectC8;
    do {
        desc.objectId = 0x138;
        desc.size = 0xE;
        coordinate = (s16 *)((u32)gOverlay60CoordsD8 + (i << 2));
        desc.x = coordinate[0];
        desc.y = coordinate[1];
        desc.z = 0;
        desc.pad0A = 0;
        desc.alpha = 0x80;
        desc.pad0C = 0;
        object = overlay60SpawnReloc(&desc, 0);
        *objects = object;
        (*object->inner)->mode = 2;
        overlay60ConfigureReloc(*objects, 0, 0, 0.0f);
        i++;
        objects++;
    } while (i < 4);

    overlay60InitReloc(gOverlay60Bss00);
    gOverlay60Bss20 = *(u32 *)(gOverlay60PostSourceReloc + 0x1E4);
    gOverlay60Bss40 = *(u32 *)(gOverlay60PostSourceReloc + 0x0C);
    gOverlay60Bss60 = *(u32 *)(gOverlay60PostSourceReloc + 0x144);
    gOverlay60Bss80 = *(u32 *)(gOverlay60PostSourceReloc + 0x1EC);

    i = overlay60ReadyReloc();
    if (i == 1) {
        gOverlay60Data58 = 4;
        gOverlay60ObjectC8->x = (f32)gOverlay60CoordE8;
        gOverlay60ObjectC8->y = (f32)gOverlay60CoordEA;
        gOverlay60ObjectCC->x = (f32)gOverlay60CoordEC;
        gOverlay60ObjectCCY->y = (f32)gOverlay60CoordEE;
        gOverlay60ObjectD0->x = (f32)gOverlay60CoordF0;
        gOverlay60ObjectD0Y->y = (f32)gOverlay60CoordF2;
        gOverlay60ObjectD4->x = (f32)gOverlay60CoordF4;
        gOverlay60ObjectD4Y->y = (f32)gOverlay60CoordF6;
    } else {
        gOverlay60Data58 = 0;
    }

    overlay60SelectReloc(((u8 *)&gOverlay60Data58)[3]);
    object = overlay60FindReloc(((u8 *)&gOverlay60Data58)[3]);
    if (object != 0) {
        object->flags |= 2;
    }
    gOverlay60Bss174 = 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o060/overlay60Initialize/func_overlay_060_F0000000_18B9DD8.s")
#endif
