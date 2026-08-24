#include "PR/ultratypes.h"

#define FIELD(base, type, off) (*(type *)((u8 *)(base) + (off)))

typedef struct Overlay65SpawnRecord {
    f32 x[9];
    f32 y[9];
    f32 z[9];
    f32 random0;
    f32 random1;
    f32 random2;
    f32 height;
    u8 active;
    u8 red;
    u8 green;
    u8 blue;
} Overlay65SpawnRecord;

typedef struct Overlay65Camera {
    u8 pad00[0x10];
    f32 y;
} Overlay65Camera;

extern u8 D_0[];
extern u8 D_1C0[];
extern s32 D_218;
extern s32 D_21C;
extern Overlay65Camera *o65GetCamera(s32 index);
extern s32 o65RandomRange(s32 low, s32 high);

#ifdef NON_MATCHING
void func_overlay_065_F0001A14_18C5C7C(f32 baseX, f32 baseY, f32 baseZ) {
    Overlay65Camera *camera;
    void *record;
    s32 i;
    s32 j;
    s32 keepGoing;
    void *point;
    u8 *colors;

    camera = o65GetCamera(0);
    record = D_0;
    i = 0;
    do {
        j = 0;
        point = record;
        if (FIELD(record, u8, 0x7C) == 0) {
            FIELD(record, u8, 0x7C) = 1;
            for (;;) {
                f32 value = (f32)j;
                j++;
                keepGoing = j < 9;
                point = (u8 *)point + 4;
                FIELD(point, f32, -4) = value + baseX;
                FIELD(point, f32, 0x20) = value + baseY;
                FIELD(point, f32, 0x44) = value + baseZ;
                if (keepGoing != 0) {
                    continue;
                }
                break;
            }
            FIELD(record, f32, 0x6C) = (f32)o65RandomRange(-D_218, D_218);
            FIELD(record, f32, 0x70) = (f32)o65RandomRange(D_21C, D_21C + 5);
            FIELD(record, f32, 0x74) = (f32)o65RandomRange(-D_218, D_218);
            colors = &D_1C0[o65RandomRange(0, 6) * 3];
            FIELD(record, u8, 0x7D) = colors[0];
            FIELD(record, u8, 0x7E) = colors[1];
            FIELD(record, u8, 0x7F) = colors[2];
            FIELD(record, f32, 0x78) = camera->y - 200.0f;
            i = 50;
        }
        i++;
        record = (u8 *)record + 0x80;
    } while (i < 50);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o065/overlay65SpawnRecord/func_overlay_065_F0001A14_18C5C7C.s")
#endif
