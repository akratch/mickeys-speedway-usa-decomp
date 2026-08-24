#include "PR/ultratypes.h"

typedef struct O8P4CF0Vec3f {
    f32 x;
    f32 y;
    f32 z;
} O8P4CF0Vec3f;

typedef struct O8P4CF0Normal {
    volatile f32 x;
    f32 y;
    f32 z;
} O8P4CF0Normal;

typedef struct O8P4CF0Actor {
    s16 angle000;
    s16 angle002;
    s16 angle004;
    u8 pad006[6];
    f32 x00C;
    f32 y010;
    f32 z014;
    u8 pad018[8];
    f32 vertical020;
} O8P4CF0Actor;

typedef struct O8P4CF0State {
    u8 pad000[4];
    f32 motion004;
    u8 pad008[0x16A];
    s8 timer172;
    u8 activated173;
    f32 blend174;
    f32 height178;
    f32 derived17C;
} O8P4CF0State;

typedef struct O8P4CF0Bounds {
    s16 minX000;
    s16 unused002;
    s16 minZ004;
    s16 extentX006;
    s16 extentZ008;
} O8P4CF0Bounds;

typedef struct O8P4CF0SceneItem {
    u8 pad000[0x44];
    s16 category044;
    u8 pad046[0x3E];
    O8P4CF0Bounds *bounds084;
    s32 callbackGate088;
} O8P4CF0SceneItem;

extern s32 O8P4CF0_call_4D14(s32 selector);
extern O8P4CF0SceneItem **O8P4CF0_call_4D24(s32 *start, s32 *end);
extern void O8P4CF0_call_4D54(s32 mode, O8P4CF0Actor *actor,
                              O8P4CF0Vec3f *input, O8P4CF0Vec3f *output);
extern void O8P4CF0_call_4E50(O8P4CF0SceneItem *item);
extern f32 O8P4CF0_call_4E64(O8P4CF0Bounds *bounds, f32 x, f32 z,
                             O8P4CF0Normal *normal);
extern f32 O8P4CF0_call_4E9C(s32 angle);
extern f32 O8P4CF0_call_4EAC(s32 angle);
extern s32 O8P4CF0_call_4EE8(f32 horizontal, f32 vertical);
extern s32 O8P4CF0_call_4EF8(f32 horizontal, f32 vertical);
extern f32 O8P4CF0_call_4F0C(f32 amount, s32 updateRate);
extern s32 O8P4CF0_call_4F34(s32 current, s32 target, f32 factor);
extern f32 O8P4CF0_call_4F48(f32 amount, s32 updateRate);
extern s32 O8P4CF0_call_4F68(s32 current, s32 target, f32 factor);

extern const f32 O8P4CF0_data_290;
extern const f32 O8P4CF0_data_294;
extern const f32 O8P4CF0_data_298;
extern const f32 O8P4CF0_data_29C;
extern const f32 O8P4CF0_data_2A0;
extern const f32 O8P4CF0_data_2A4;
extern const f32 O8P4CF0_data_2A8;
extern f32 O8P4CF0_data_4FD4;

void func_overlay_008_F0004CF0_1862A48(O8P4CF0Actor *actor,
                                       O8P4CF0State *state,
                                       s32 updateRate) {
    s32 start;
    s32 end;
    O8P4CF0SceneItem **items;
    register f32 motionTarget;
    f32 blendFactor;
    s32 targetB;
    register f32 horizontalA;
    f32 horizontalB;
    register f32 axisA;
    register f32 axisB;
    register f32 surfaceHeight;
    O8P4CF0Vec3f point;
    O8P4CF0Normal normal;

    state->activated173 = 0;

    if (O8P4CF0_call_4D14(20) != 0) {
        items = O8P4CF0_call_4D24(&start, &end);
        point.x = 0.0f;
        point.y = 0.0f;
        point.z = 7.0f;
        O8P4CF0_call_4D54(1, actor, &point, &point);
        point.x += actor->x00C;
        surfaceHeight = (point.y += actor->y010);
        point.z += actor->z014;
        surfaceHeight = point.y;

        if (start < end) {
            do {
                O8P4CF0SceneItem *item = items[start++];

                if (item->category044 == 0x3D) {
                    O8P4CF0Bounds *bounds = item->bounds084;

                    if (bounds != 0) {
                        if ((bounds->minX000 <= (s32)point.x) &&
                            ((s32)point.x <=
                             bounds->minX000 + bounds->extentX006)) {
                            if ((bounds->minZ004 <= (s32)point.z) &&
                                ((s32)point.z <=
                                 bounds->minZ004 + bounds->extentZ008)) {
                                if (item->callbackGate088 != 0) {
                                    O8P4CF0_call_4E50(item);
                                }
                                surfaceHeight = O8P4CF0_call_4E64(
                                    bounds, point.x, point.z, &normal);
                                break;
                            }
                        }
                    }
                }
            } while (start < end);
        }

        if (surfaceHeight > point.y) {
            s32 targetA;
            f32 factor;

            axisA = O8P4CF0_call_4E9C(-actor->angle000);
            axisB = O8P4CF0_call_4EAC(-actor->angle000);
            horizontalB = normal.x;
            horizontalA = -(normal.z * axisA + horizontalB * axisB);
            horizontalB = normal.z * axisB - horizontalB * axisA;
            targetA = O8P4CF0_call_4EE8(horizontalA, normal.y);
            targetB = O8P4CF0_call_4EF8(horizontalB, normal.y);

            factor = O8P4CF0_call_4F0C(0.9f, updateRate);
            actor->angle004 = (s16)O8P4CF0_call_4F34(
                actor->angle004, (s16)targetA, 1.0f - factor);
            factor = O8P4CF0_call_4F48(0.9f, updateRate);
            actor->angle002 = (s16)O8P4CF0_call_4F68(
                actor->angle002, (s16)targetB, 1.0f - factor);

            start = updateRate - 1;
            if (updateRate != 0) {
                do {
                    actor->vertical020 +=
                        (((surfaceHeight - state->height178 - point.y) *
                          0.5f) - actor->vertical020) * 0.25f;
                } while (start--);
            }

            O8P4CF0_data_4FD4 *= 0.4f;
            state->timer172 = 12;
            state->activated173 = 1;
        }
    }

    if (state->timer172 != 0) {
        state->timer172 =
            (s8)((u32)(s32)state->timer172 - (u32)updateRate);
        if (state->timer172 <= 0) {
            state->timer172 = 0;
            state->blend174 = (f32)1;
            state->height178 = 2.0f;
            state->derived17C = 1.0f;
            return;
        } else {
            s32 count;

            axisA = state->motion004;
            count = updateRate - 1;
            if (axisA < 0.0f) {
                axisA = -axisA;
            }
            axisA *= 0.2f;
            if (1.0f < axisA) {
                axisA = 1.0f;
            }

            start = count;
            if (count != -1) {
                blendFactor = 0.05f;

                do {
                    state->blend174 +=
                        (axisA - state->blend174) * blendFactor;
                } while (start--);
            }
            state->height178 = state->blend174 * -4.0f + 6.0f;
            state->derived17C =
                state->blend174 * 0.024999976f + 0.975f;
        }
    }
}
