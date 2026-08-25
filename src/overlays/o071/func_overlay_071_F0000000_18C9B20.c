#include "PR/ultratypes.h"

typedef struct O71Input {
    u8 pad00[0xA];
    u16 value0A;
    u16 value0C;
    s16 value0E;
    s16 value10;
    s16 value12;
    u8 value14;
    u8 value15;
    u8 value16;
    u8 value17;
    u8 value18;
    u8 value19;
    u16 value1A;
    u8 value1C;
    u8 value1D;
    u8 value1E;
    s8 value1F;
    s16 value20;
    s16 value22;
} O71Input;

typedef struct O71Color {
    u8 pad00[6];
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
} O71Color;

typedef struct O71Data {
    O71Color colors[2][8];
    f32 valueA0;
    f32 valueA4;
    f32 valueA8;
    f32 valueAC;
    f32 valueB0;
    f32 valueB4;
    f32 valueB8;
    f32 valueBC;
    f32 valueC0;
    f32 valueC4;
    u8 valueC8;
    u8 valueC9;
    u8 valueCA;
    u8 valueCB;
    u8 valueCC;
    u8 valueCD;
    u16 valueCE;
} O71Data;

typedef struct O71Object {
    s16 value00;
    s16 value02;
    s16 value04;
    u8 pad06[2];
    f32 value08;
    u8 pad0C[0x58];
    O71Data *data;
} O71Object;

extern const f32 gOverlay71Scale0;
extern const f32 gOverlay71Scale1;
extern void func_overlay_071_F0000278_18C9D98(O71Object *object);

/* Mickey-local reconstruction; no external donor body was used. */
void func_overlay_071_F0000000_18C9B20(O71Object *object, O71Input *input,
                                       s32 preserveState) {
    O71Data *data;
    O71Color *color;
    s32 group;
    s32 index;
    f32 scale;

    object->value00 = input->value22;
    data = object->data;
    object->value02 = input->value20;
    object->value04 = input->value1F << 6;
    object->value08 = gOverlay71Scale0;

    data->valueA0 = (f32)input->value0E * 10.0f;
    data->valueA4 = (f32)input->value10 * 10.0f;
    data->valueA8 = (f32)input->value12 * 10.0f;
    data->valueAC = (f32)input->value0A * 10.0f;
    data->valueB0 = (f32)input->value0C * 10.0f;
    data->valueB4 = (f32)input->value1A * 10.0f;
    scale = gOverlay71Scale1;
    data->valueB8 = (f32)input->value1C * scale;
    data->valueBC = (f32)input->value1D * scale;
    data->valueC0 = (f32)input->value14 * scale;
    data->valueC4 = (f32)input->value15 * scale;
    data->valueC8 = input->value16;
    data->valueC9 = input->value17;
    data->valueCA = input->value18;
    data->valueCB = input->value19;
    data->valueCC = input->value1E;

    for (group = 0; group < 2; group++) {
        index = 0;
        color = data->colors[group];
        do {
            if (index < 4) {
                color->red = 0xFF;
                color->green = 0xFF;
                color->blue = 0xFF;
                color->alpha = input->value19;
            } else {
                color->red = input->value16;
                color->green = input->value17;
                color->blue = input->value18;
                color->alpha = 0;
            }
            color++;
            index++;
        } while (index != 8);
    }
    if (preserveState == 0) {
        data->valueCD = 0;
        data->valueCE = 0;
    }
    func_overlay_071_F0000278_18C9D98(object);
}
