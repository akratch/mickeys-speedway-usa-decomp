#include "PR/ultratypes.h"

/* Typed semantic baseline from preserved Epoch11 scaffold for overlay 57 +0x0954..+0x1020. */

extern u8 O57_D_1A8;
extern s32 O57_D_104;
extern s32 O57_D_114;
extern s32 O57_D_118;
extern s32 O57_D_128;
extern s32 O57_D_13C;
extern s32 O57_D_140;
extern s32 O57_D_160;
extern s32 O57_D_164;
extern s32 O57_D_184;
extern s32 O57_D_188;
extern s32 O57_D_194;
extern s32 O57_D_198;
extern s32 O57_D_19C;
extern s32 O57_D_224;
extern s32 O57_D_244;
extern s32 O57_D_264;
extern s32 O57_D_4CC;

typedef struct O57Config0954 {
    u8 pad00[8];
    u16 field08;
    u16 field0A;
    u16 field0C;
} O57Config0954;

typedef struct O57RenderChannel0954 {
    void *object00;
    s32 pad04;
    s32 fixed08;
} O57RenderChannel0954;

typedef struct O57Metadata0954 {
    u8 pad00[0x2C];
    u8 count2C;
} O57Metadata0954;

typedef struct O57Entry0954 {
    u16 value00;
    u8 pad02[2];
    u32 flags04;
} O57Entry0954;

typedef struct O57Owner0954 {
    O57Metadata0954 *metadata00;
    u8 pad04[0x48];
    O57Entry0954 *entries4C;
} O57Owner0954;

typedef struct O57Level2_0954 {
    O57Owner0954 *owner00;
} O57Level2_0954;

typedef struct O57Level1_0954 {
    u8 pad00[0x68];
    O57Level2_0954 *level68;
} O57Level1_0954;

typedef struct O57Lookup0954 {
    u8 pad00[8];
    O57Level1_0954 *level08;
} O57Lookup0954;

extern O57Config0954 O57_config0954;
extern s32 O57_mode0954;
extern u32 O57_nibbleFlags0954;
extern u32 O57_node30Flags0954;
extern s8 O57_labels390[12];
extern s8 O57_labels39CEnd;
extern s8 O57_labels39CStart[4];
extern s8 O57_labels3A0End;
extern s8 O57_labels3A0Start[4];
extern s8 O57_labels3A4;
extern void *O57_queryObject0954;
extern u8 O57_renderObject0954[];
extern u8 O57_renderData21C[];
extern u8 O57_renderData23C[];
extern O57RenderChannel0954 O57_renderChannel1FC;
extern u8 O57_channelData4D0[];
extern f32 O57_channelValue4DC;
extern u8 O57_channelData4D8[];
extern f32 O57_channelValue4E0;

extern void O57_call_096C(void *state, s32 updateRate);
extern void O57_call_1AE8(s32 updateRate);
extern void O57_call_0990(void *object);
extern s32 O57_call_0998(void);
extern void O57_call_09D0(s32 value);
extern void O57_call_09E0(void);
extern void O57_call_0A20(void);
extern u8 *O57_call_0A28(s32 id);
extern void O57_call_0A4C(void *child, s32 zero, s32 negativeOne,
                          f32 one);
extern void O57_call_0AA4(void);
extern void O57_call_0B78(s32 notification);
extern void O57_call_0C04(s32 notification);
extern void O57_call_0CE8(s32 notification);
extern O57Lookup0954 *O57_call_0DA4(s32 id);
extern void O57_call_0E80(void *object, void *data, f32 x, f32 y,
                          f32 oneA, f32 oneB, s32 negativeTwo, s32 flags);
extern void O57_call_0EC0(void *object, void *data, f32 x, f32 y,
                          f32 oneA, f32 oneB, s32 negativeTwo, s32 flags);
extern void O57_call_0F14(void *object, void *data, f32 x, f32 y,
                          f32 oneA, f32 oneB, s32 negativeTwo, s32 flags);
extern void O57_call_0F68(void *object, void *data, f32 x, f32 y,
                          f32 oneA, f32 oneB, s32 negativeTwo, s32 flags);
extern void O57_call_0F8C(void *object, void *data, s32 count, f32 *value,
                          s32 updateRate);
extern void O57_call_0FD0(void *object, void *data, s32 count, f32 *value,
                          s32 updateRate);
extern void overlay57ApplyValue(s32 node, s32 label, s32 value);

#define O57_APPLY_FIXED_LIST_0954() \
    do { \
        overlay57ApplyValue(0, 10, 1); \
        overlay57ApplyValue(2, 9, 1); \
        overlay57ApplyValue(3, 10, 1); \
        overlay57ApplyValue(6, 4, 1); \
        overlay57ApplyValue(19, 4, 1); \
        overlay57ApplyValue(23, 0, 1); \
        overlay57ApplyValue(28, 3, 1); \
        overlay57ApplyValue(4, 5, 1); \
        overlay57ApplyValue(7, 9, 1); \
        overlay57ApplyValue(13, 0, 1); \
        overlay57ApplyValue(23, 3, 1); \
        overlay57ApplyValue(25, 2, 1); \
        overlay57ApplyValue(29, 0, 1); \
    } while (0)

void func_overlay_057_F0000954_18A454C(s32 updateRate) {
    s8 *cursor;
    s32 fixed;

    O57_call_096C(&O57_D_1A8, updateRate);
    O57_call_1AE8(updateRate);

    if (O57_D_4CC == 0) {
        s32 query;
        O57Lookup0954 *lookup;

        O57_call_0990(O57_queryObject0954);
        query = O57_call_0998();

        switch (query) {
        case 10:
            O57_call_09D0(0);
            break;
        case 12:
            O57_call_09E0();
            break;
        case 17: {
            u8 *entry;

            O57_D_194 = 0;
            O57_D_118 = 10;
            O57_D_164 = 0;
            O57_D_160 = 0;
            O57_D_184 = 47;
            O57_D_114 = 180;
            O57_call_0A20();
            entry = O57_call_0A28(47);
            if ((entry != 0) && (*(void **)(entry + 8) != 0)) {
                O57_call_0A4C(*(void **)(entry + 8), 0, -1, 1.0f);
            }
            O57_D_188 = 1;
            O57_D_13C = 255;
            O57_D_140 = 0;
            break;
        }
        case 18:
            O57_D_118 = 20;
            O57_D_164 = 0;
            O57_D_160 = 0;
            O57_D_184 = 75;
            O57_D_114 = 180;
            O57_call_0AA4();
            O57_D_13C = 255;
            O57_D_140 = 0;
            break;
        }

        if (O57_config0954.field08) {}

        if (((((O57_config0954.field08 & 0xFFFF) & 0x1C0) >> 6) >= 3) &&
            (((O57_config0954.field0A & 0x1C0) >> 6) >= 3) &&
            (((O57_config0954.field0C & 0x1C0) >> 6) >= 3)) {
            O57_D_198 = 1;
            if (O57_mode0954 == 1) {
                overlay57ApplyValue(8, 0, 1);
                overlay57ApplyValue(31, 0, 1);
            } else {
                overlay57ApplyValue(8, 0, 2);
                overlay57ApplyValue(31, 0, 2);
            }
        } else {
            O57_call_0B78(3);
        }

        if (((O57_nibbleFlags0954 << 5) >> 28) == 15) {
            O57_D_19C = 1;
            if (O57_mode0954 == 1) {
                overlay57ApplyValue(3, 11, 1);
                overlay57ApplyValue(22, 3, 1);
            } else {
                overlay57ApplyValue(3, 11, 2);
                overlay57ApplyValue(22, 3, 2);
            }
        } else {
            O57_call_0C04(4);
        }

        if (O57_mode0954 != 1) {
            O57_APPLY_FIXED_LIST_0954();
            O57_call_0CE8(7);
        }

        cursor = O57_labels390;
        do {
            overlay57ApplyValue(47, *cursor, 1);
            cursor++;
        } while (cursor < &O57_labels39CEnd);
        cursor = O57_labels39CStart;
        do {
            overlay57ApplyValue(47, *cursor, O57_D_198);
            cursor++;
        } while (cursor < &O57_labels3A0End);
        cursor = O57_labels3A0Start;
        do {
            overlay57ApplyValue(47, *cursor, O57_D_19C);
            cursor++;
        } while (cursor < &O57_labels3A4);
        overlay57ApplyValue(47, 30, (O57_node30Flags0954 << 13) >> 31);

        lookup = O57_call_0DA4(75);
        if (lookup != 0) {
            O57Owner0954 *owner = lookup->level08->level68->owner00;

            if (owner != 0) {
                O57Entry0954 *entries = owner->entries4C;

                if (entries != 0) {
                    u32 count = owner->metadata00->count2C;
                    s32 oldCount;

                    oldCount = count;
                    count--;
                    if (oldCount != 0) {
entry_loop:
                            if ((entries->flags04 & 0x00100000) != 0) {
                                entries->value00 = 0x0100;
                            }
                            entries++;
                        oldCount = count;
                        count--;
                        if (oldCount != 0) {
                            goto entry_loop;
                        }
                    }
                }
            }
        }
        O57_D_4CC = 1;
    }

    {
        O57_D_104 += updateRate << 4;
        if (O57_D_104 >= 256) {
            O57_D_104 = 255;
        }
    }

    O57_call_0E80(O57_renderObject0954, O57_renderData21C,
                  0.0f, 0.0f, 1.0f, 1.0f, -2, 3);
    O57_call_0EC0(O57_renderObject0954, O57_renderData23C,
                  0.0f, 0.0f, 1.0f, 1.0f, -2, 3);
    O57_call_0F14(O57_renderObject0954, &O57_renderChannel1FC,
                  (f32)O57_D_128, 0.0f, 1.0f, 1.0f, -2, 3);
    O57_call_0F68(O57_renderObject0954, &O57_renderChannel1FC,
                  (f32)(320 - O57_D_128), 0.0f,
                  1.0f, 1.0f, -2, 0x1003);

    O57_call_0F8C(O57_renderChannel1FC.object00, O57_channelData4D0, 12,
                  &O57_channelValue4DC, updateRate);
    O57_renderChannel1FC.fixed08 = (s32)(O57_channelValue4DC * 65536.0f);
    O57_call_0FD0(O57_renderChannel1FC.object00, O57_channelData4D8, 2,
                  &O57_channelValue4E0, updateRate);

    fixed = (s32)(O57_channelValue4E0 * 65536.0f);
    O57_D_224 = fixed;
    O57_D_244 = fixed;
    O57_D_264 = fixed;
}
