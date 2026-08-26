#include "PR/ultratypes.h"

typedef struct O50Record {
    u8 pad00[0x08];
    s32 value08;
    s16 value0C;
    s16 value0E;
    u8 pad10[0x08];
    s32 value18;
    s16 value1C;
    s16 value1E;
    u8 pad20[0x08];
    s32 value28;
    s16 value2C;
    s16 value2E;
    u8 pad30[0x08];
    s32 value38;
    s16 value3C;
    s16 value3E;
} O50Record;

typedef struct O50Config {
    u8 pad00[0x04];
    s16 value04;
    u8 pad06[0x22];
    f32 value28;
    f32 value2C;
    u8 pad30[0x5C];
    f32 value8C;
} O50Config;

typedef struct O50Locals {
    u8 *savedFlag;
    u32 unused;
} O50Locals;

extern s16 D_0;
extern s32 D_8;
extern s16 D_C;
extern s16 D_E;
extern O50Record D_10[2];
extern u8 D_48;
extern u8 D_6C;
extern s32 D_A0;
extern f32 D_A4;
extern f32 D_A8;
extern s32 D_AC;
extern s32 D_B0[3];
extern s32 D_BC;
extern s32 D_C0;
extern void *D_C4;
extern s16 D_C8;
extern s16 D_CA;
extern s32 D_DC;
extern O50Record D_FC;
extern u8 D_12C;
extern u8 D_1CC;
extern s32 D_228;
extern s16 D_22C;
extern s16 D_22E;
extern O50Record D_230[2];
extern u8 D_2B0;
extern u8 D_2C0;
extern u8 D_2E0;
extern u8 D_300;
extern s8 D_330;
extern O50Config gO50Config;
extern f32 gO50FloatSource;

extern u8 *o50AcquireFlagReloc();
extern void o50LoadReloc();
extern void o50ConfigureReloc();
extern void o50SetModeReloc();
extern void o50EnableReloc();
extern void overlay50PatchIndices(void *entry);
extern void o50PrepareReloc();
extern s16 o50ReadValueReloc();
extern u8 *o50GetStateReloc();
extern void o50SelectReloc();
extern void *o50CreateA(void);
extern void *o50CreateB(void *value);
extern void *o50CreateC(void *value);
extern void *o50CreateD(void *value, s32 x, s32 y, s32 count);
extern void o50FinalizeReloc(void *value, void *arg);

/* Workbench plateau: structure-mismatch, 108 words; 185/185 instructions, frame -32, first +0x8.
 * Levers: config-base, record initialization/copy order, flags, register/placement, and empty-read variants.
 * Remains: config-base coloring, record-copy schedule, and overlay relocation identity. */
#ifdef NON_MATCHING
void func_overlay_050_F0000000_1896970(void) {
    volatile O50Locals locals;
    s32 *fill;
    O50Record *source;
    O50Record *dest;
    O50Record *end;
    O50Config *config;
    u8 *state;
    void *object;

    locals.savedFlag = o50AcquireFlagReloc();
    o50LoadReloc(&D_0);
    o50ConfigureReloc(&D_48);
    o50SetModeReloc(4);
    D_A0 = 0x104;
    o50EnableReloc(0xB);

    overlay50PatchIndices(&D_6C);
    overlay50PatchIndices(&D_12C);
    overlay50PatchIndices(&D_AC);
    overlay50PatchIndices(&D_DC);
    overlay50PatchIndices(&D_FC);
    overlay50PatchIndices(&D_300);
    overlay50PatchIndices(&D_2C0);
    overlay50PatchIndices(&D_2E0);
    overlay50PatchIndices(&D_1CC);

    config = &gO50Config;
    D_A4 = -100.0f;
    D_A8 = 120.0f;
    config->value8C = 42.0f;
    config->value2C = -78.0f;
    config->value04 = 0x4000;
    config->value28 = gO50FloatSource;
    fill = D_B0;
    do {
        *fill++ = 0xA0;
    } while (fill < (s32 *)&D_BC);

    o50PrepareReloc();
    D_330 = -1;
    D_0 = o50ReadValueReloc();
    D_BC = -0x500;
    D_C0 = -0x140;
    D_8 = D_228;
    D_E = D_22E;
    D_C = D_22C;

    source = D_230;
    dest = D_10;
    end = (O50Record *)&D_2B0;
    do {
        dest->value0C = source->value0C;
        dest->value0E = source->value0E;
        dest->value1C = source->value1C;
        dest->value38 = source->value38;
        dest->value3E = source->value3E;
        dest->value1E = source->value1E;
        dest->value18 = source->value18;
        dest->value2C = source->value2C;
        dest->value2E = source->value2E;
        dest->value28 = source->value28;
        dest->value3C = source->value3C;
        dest->value08 = source->value08;
        source++;
        dest++;
    } while (source != end);

    state = o50GetStateReloc();
    if (*state == 1) {
        config->value2C += -40.0f;
        D_FC.value0C -= 0x28;
        D_FC.value1C -= 0x28;
    }

    if (*locals.savedFlag != 0) {
        D_C4 = NULL;
    } else {
        o50SelectReloc(3);
        object = o50CreateD(o50CreateB(o50CreateA()),
                            0xA0, 0x78, 0xC);
        D_C4 = object;
        o50FinalizeReloc(object, NULL);
    }
    D_CA = 0;
    D_C8 = 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o050/overlay50Initialize/func_overlay_050_F0000000_1896970.s")
#endif
