#include "PR/ultratypes.h"

typedef struct O8P2640Anchor {
    s16 helperInput0;
    u8 pad002[0x0A];
    f32 coordC;
    f32 coord10;
    f32 coord14;
} O8P2640Anchor;

typedef struct O8P2640Config {
    u8 pad00;
    s8 tuningIndex1;
} O8P2640Config;

typedef struct O8P2640Tuning {
    f32 extent0;
    f32 offset4;
    f32 spreadScale8;
} O8P2640Tuning;

typedef struct O8P2640Record {
    s16 value0;
    s16 value2;
    f32 magnitude4;
    f32 coord8;
    f32 coordC;
    f32 coord10;
    f32 phase14;
    s16 size18;
    u8 kind1A;
    u8 pad1B;
    u32 packed1C;
    u32 packed20;
    u32 packed24;
    u32 packed28;
    u32 packed2C;
    u32 packed30;
} O8P2640Record;

extern const O8P2640Tuning D_2110[];
extern const f32 O8P2640_data_198;
extern const f32 O8P2640_data_19C;
extern f32 O8P2640_call_26AC(f32 squaredDistance);
extern s32 O8P2640_call_26F0(f32 negX, f32 negZ);
extern f32 O8P2640_call_26FC(s32 anchorHalfword);
extern f32 O8P2640_call_2708(s32 anchorHalfword);
extern s32 O8P2640_call_27BC(s32 low, s32 high);
extern s32 O8P2640_call_27CC(s32 low, s32 high);
extern s32 O8P2640_call_27DC(s32 low, s32 high);
extern void O8P2640_call_28C0(O8P2640Record *record);

#ifdef NON_MATCHING
void func_overlay_008_F0002640_1860398(
    O8P2640Anchor *anchor, O8P2640Config *config, s32 orientation,
    s32 randomLow, s32 randomHigh, f32 distanceX, f32 unusedStackFloat,
    f32 distanceZ, s32 emissionCount) {
    O8P2640Record record;
    const O8P2640Tuning *tuning;
    f32 axisA;
    f32 axisB;
    f32 clampedDistance;
    f32 spread;
    f32 magnitudeScale;
    s32 baseValue;
    s32 randomOffset;
    s32 randomValue;
    s32 magnitudeValue;
    s32 tuningIndex;

    (void)unusedStackFloat;
    tuningIndex = config->tuningIndex1;
    if (tuningIndex >= 10) {
        tuningIndex = 0;
    }

    clampedDistance = O8P2640_call_26AC(
        distanceX * distanceX + distanceZ * distanceZ);
    if (clampedDistance < 2.0f) {
        return;
    }
    if (clampedDistance > 8.0f) {
        clampedDistance = 8.0f;
    }

    baseValue = O8P2640_call_26F0(-distanceX, -distanceZ);
    axisA = O8P2640_call_26FC(anchor->helperInput0);
    axisB = O8P2640_call_2708(anchor->helperInput0);
    tuning = &D_2110[tuningIndex];

    record.coordC = anchor->coord10 + tuning->offset4;
    record.phase14 = O8P2640_data_198;
    record.size18 = 0x80;
    record.kind1A = 5;
    record.packed1C = 0xFFFFFFFF;
    record.packed28 = 0xFFFF80FF;
    record.packed20 = 0xFFFF80E0;
    record.packed2C = 0xFF8000E0;
    record.packed24 = 0xFF800000;
    record.packed30 = 0xFF000000;

    if (emissionCount == 0) {
        return;
    }
    emissionCount--;
    magnitudeScale = O8P2640_data_19C;
    do {
        randomOffset = O8P2640_call_27BC(-0xC80, 0xC80);
        randomValue = O8P2640_call_27CC(randomLow, randomHigh);
        magnitudeValue = O8P2640_call_27DC(0x50, 0x78);
        record.value0 = (s16)(baseValue + randomOffset);
        record.value2 = (s16)randomValue;
        record.magnitude4 = (f32)magnitudeValue *
                            clampedDistance * magnitudeScale;

        spread = (f32)randomOffset * tuning->spreadScale8;
        if (orientation == 0) {
            record.coord8 = anchor->coordC - tuning->extent0 * axisB +
                            spread * axisA;
            record.coord10 = anchor->coord14 + spread * axisB +
                             tuning->extent0 * axisA;
        } else if (orientation == 2) {
            record.coord8 = anchor->coordC + tuning->extent0 * axisB +
                            spread * axisA;
            record.coord10 = anchor->coord14 + spread * axisB -
                             tuning->extent0 * axisA;
        } else {
            record.coord8 = anchor->coordC + spread * axisB;
            record.coord10 = anchor->coord14 - spread * axisA;
        }

        O8P2640_call_28C0(&record);
        record.phase14 = 1.0f;
    } while (emissionCount--);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o008/func_overlay_008_F0002640_1860398/func_overlay_008_F0002640_1860398.s")
#endif
