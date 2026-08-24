#include "PR/ultratypes.h"

typedef struct Overlay41CurveNode {
    s16 x;
    s16 y;
    s16 z;
    u8 reserved06;
    u8 linear;
    f32 value08;
    f32 value0C;
    f32 value10;
    f32 value14;
    u8 reserved18[0x0C];
    struct Overlay41CurveNode *previous;
    struct Overlay41CurveNode *next;
} Overlay41CurveNode;

/* PROVENANCE: Diddy Kong Racing, src/objects.c
 * (cubic_spline_interpolation), and Jet Force Gemini,
 * src/hasm/ido/math_util.s (splinePos); declaration and constant-family
 * source-shape analogues only. Mickey's ROM decides every detail. */
#ifdef NON_MATCHING
void func_overlay_041_F00002AC_18875E4(Overlay41CurveNode *node, f32 t,
                                        f32 *valueOut, f32 *tangentOut,
                                        s32 component) {
    Overlay41CurveNode *p0;
    Overlay41CurveNode *p2;
    Overlay41CurveNode *p3;
    f32 difference;
    volatile f32 ret;
    f32 coefficient0;
    f32 coefficient1;
    f32 coefficient2;
    f32 values[4];
    s32 angle;
    s32 i;
    s32 j;


    p0 = node->previous;
    p2 = node->next;
    p3 = p2->next;
    angle = 0;

    switch (component) {
        case 0:
            values[0] = p0->value0C;
            values[1] = node->value0C;
            values[2] = p2->value0C;
            values[3] = p3->value0C;
            break;
        case 1:
            values[0] = p0->value10;
            values[1] = node->value10;
            values[2] = p2->value10;
            values[3] = p3->value10;
            break;
        case 2:
            values[0] = p0->value14;
            values[1] = node->value14;
            values[2] = p2->value14;
            values[3] = p3->value14;
            break;
        case 3:
            angle = 1;
            values[0] = p0->x;
            values[1] = node->x;
            values[2] = p2->x;
            values[3] = p3->x;
            break;
        case 4:
            angle = 1;
            values[0] = p0->y;
            values[1] = node->y;
            values[2] = p2->y;
            values[3] = p3->y;
            break;
        case 5:
            angle = 1;
            values[0] = p0->z;
            values[1] = node->z;
            values[2] = p2->z;
            values[3] = p3->z;
            break;
        default:
            values[0] = p0->value08;
            values[1] = node->value08;
            values[2] = p2->value08;
            values[3] = p3->value08;
            break;
    }

    if (angle != 0) {
        for (i = 0; i < 3; i++) {
            difference = values[i + 1] - values[i];
            if (difference > 32768.0f) {
                for (j = i + 1; j < 4; j++) {
                    values[j] -= 65536.0f;
                }
            } else if (difference < -32768.0f) {
                for (j = i + 1; j < 4; j++) {
                    values[j] += 65536.0f;
                }
            }
        }
    }

    if (node->linear != 0) {
        if (valueOut != 0) {
            *valueOut = values[1] + ((values[2] - values[1]) * t);
        }
        if (tangentOut != 0) {
            *tangentOut = values[2] - values[1];
        }
    } else {
        ret = values[1];
        coefficient0 = (-0.56f * values[0]) + (0.56f * values[2]);
        coefficient1 = (values[3] * -0.56f) +
                       (1.12f * values[0]) + (-2.44f * values[1]) +
                       (1.88f * values[2]);
        coefficient2 = (values[3] * 0.56f) + (-0.56f * values[0]) +
                       (1.44f * values[1]) + (-1.44f * values[2]);
        if (valueOut != 0) {
            *valueOut = (((coefficient2 * t) + coefficient1) * t +
                         coefficient0) * t + ret;
        }
        if (tangentOut != 0) {
            *tangentOut = ((((3.0f * coefficient2) * t) +
                            (2.0f * coefficient1)) * t) + coefficient0;
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o041/overlay41SampleCurve/func_overlay_041_F00002AC_18875E4.s")
#endif
