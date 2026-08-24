#include "overlays/overlay_001.h"

/* ---- overlay1ResolvePathPoint ---- */


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
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o001/overlay_001_end/func_overlay_001_F0007D6C_185414C.s")
#endif

/* ---- overlay1ModeChecks ---- */


/* Generic scalar predicates only; pinned DKR objects provide no donor. */
extern s32 gOverlay1Mode;

s32 overlay1ModeIsOne(void) { return gOverlay1Mode == 1; }
s32 overlay1ModeIsTwo(void) { return gOverlay1Mode == 2; }
s32 overlay1ModeIsThree(void) { return gOverlay1Mode == 3; }

/* ---- overlay1DistanceFromCurrent ---- */


/* Pinned DKR v77/v80 and JFG scans classify overlay 1 as no donor. */
extern void *gOverlay1DistanceObject;
extern f32 overlay1DistanceReloc(void *first, void *second);

f32 overlay1DistanceFromCurrent(void *other) {
    void *current;

    current = gOverlay1DistanceObject;
    if (current != NULL) {
        return overlay1DistanceReloc(current, other);
    }
    return 0.0f;
}

/* ---- overlay1DistanceFromSelected ---- */


/* Pinned DKR v77/v80 and JFG scans classify overlay 1 as no donor. */
extern void *gOverlay1DistanceObject;
extern u8 gOverlay1SelectedIndex;
extern void **overlay1GetSelectionReloc(s32 *count);
extern f32 overlay1DistanceReloc(void *first, void *second);

f32 overlay1DistanceFromSelected(void *object) {
    s32 count;
    void **objects;
    volatile s32 reservation[2];

    if (object == gOverlay1DistanceObject) {
        objects = overlay1GetSelectionReloc(&count);
        if (count >= 2) {
            return overlay1DistanceReloc(object, objects[gOverlay1SelectedIndex]);
        }
    }
    return 0.0f;
}
