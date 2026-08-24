#include "PR/ultratypes.h"

typedef struct Overlay1PoolRecord {
    s16 x[32];
    s16 y[32];
    u8 selector[32];
    u8 mode[32];
    u8 count;
    u8 type;
    u8 padC2[6];
    u32 value;
} Overlay1PoolRecord;

typedef struct Overlay1ErrorOwner {
    u8 pad0;
    s8 sound;
} Overlay1ErrorOwner;

extern Overlay1PoolRecord D_220[32];
extern Overlay1PoolRecord D_220_Clear[32];
extern u8 D_8[0x200];
extern Overlay1PoolRecord *D_218;
extern s32 D_1D88;
extern s32 D_1D84;
extern Overlay1ErrorOwner *D_1DA0;
extern void *D_1BA4;

extern void overlay1ClearReloc(void *address, s32 size);
extern Overlay1PoolRecord *func_overlay_001_F0007BDC_1853FBC(
    s16 x0, s16 y0, s16 x1, s16 y1);
extern void overlay1ErrorReloc(s32 sound);
extern s32 overlay1SegmentReloc(f32 x0, f32 y0, f32 x1, f32 y1,
                                void *world, s16 *result, s32 sentinel,
                                u32 mask);
extern void func_overlay_001_F0007730_1853B10(s16 *x, s16 *y,
                                               u8 selector, u8 mode);

#ifdef NON_MATCHING
s32 overlay1ResolvePathPoint(s16 x0, s16 y0, s16 x1, s16 y1,
                             s16 *outX, s16 *outY) {
    register u32 groupAddress;
    s32 index;
    Overlay1PoolRecord *record;
    s16 *point;
    s16 result[4];
    s32 scanIndex;
    s32 product;

    overlay1ClearReloc(D_220_Clear, sizeof(D_220_Clear));
    overlay1ClearReloc(D_8, sizeof(D_8));
    D_218 = D_220;
    groupAddress = (u32)&D_1D88;
    *(s32 *)groupAddress = 0x3F;
    D_1D84 = 0;

    record = func_overlay_001_F0007BDC_1853FBC(x0, y0, x1, y1);
    if (record == 0) {
        overlay1ErrorReloc(D_1DA0->sound);
        *outX = x0;
        *outY = y0;
        return -1;
    }

    (*(s32 *)groupAddress)--;
    index = 2;
    point = &record->x[2];
    if (record->count >= 2) {
        scanIndex = 2;
        if (record->count >= 3) {
            point = &record->x[2];
            do {
                if (overlay1SegmentReloc((f32)x0, (f32)y0,
                                         (f32)point[0], (f32)point[32],
                                         D_1BA4, result, -1, 0xFFFF) != 0) {
                    product =
                        (result[0] - point[0]) * (result[0] - point[32]);
                    product = product + product;
                    if ((u32)product >= 0x11U) {
                        break;
                    }
                }
                scanIndex++;
                point++;
            } while (scanIndex < record->count);
        }
        point = &record->x[scanIndex];
    } else {
        scanIndex = index;
        point = &record->x[index];
    }

    if (record->selector[scanIndex - 1] != 0xFF) {
        func_overlay_001_F0007730_1853B10(
            point - 1, point + 31, record->selector[scanIndex - 1],
            record->mode[scanIndex - 1]);
    }
    *outX = point[-1];
    *outY = point[31];
    return record->count;
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay1ResolvePathPoint/func_overlay_001_F0007D6C_185414C.s")
#endif
