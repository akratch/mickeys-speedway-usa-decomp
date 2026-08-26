#include "PR/ultratypes.h"

typedef struct Overlay41CurveNode {
    s16 angleX;
    s16 angleY;
    s16 angleZ;
    u8 reserved06;
    u8 linear;
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
    s16 tangentX;
    s16 tangentY;
    s16 tangentZ;
    u8 reserved1E[2];
    f32 speed;
    struct Overlay41CurveNode *previous;
    struct Overlay41CurveNode *next;
} Overlay41CurveNode;

typedef struct Overlay41ObjectExtra {
    f32 scale;
    u8 reserved04[0x50];
    f32 scaleX;
    f32 scaleY;
} Overlay41ObjectExtra;

typedef struct Overlay41Object {
    s16 angleX;
    s16 angleY;
    s16 angleZ;
    u8 reserved06[2];
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
    u8 reserved18[4];
    f32 velocityX;
    f32 velocityY;
    f32 velocityZ;
    u8 reserved28[6];
    s16 positionTag;
    u8 reserved30[0x10];
    Overlay41ObjectExtra *extra;
    u8 reserved44[8];
    f32 *scalePair;
} Overlay41Object;

typedef struct Overlay41Input {
    u8 reserved00;
    u8 targetIndex;
    u8 reserved02[6];
    Overlay41Object *object;
    f32 amount0C;
    f32 smoothing;
    u8 reserved14;
    u8 orientationMode;
    u8 flags;
    u8 reserved17;
    Overlay41CurveNode *node;
    f32 t;
    u8 reserved20[0x0A];
    s16 scaleTimer;
    f32 scaleAmount;
    f32 scaleRate;
} Overlay41Input;

typedef struct Overlay41Target {
    u8 reserved00[8];
    Overlay41Object *object;
} Overlay41Target;

extern s32 gOverlay41Disabled;
extern Overlay41Target **gOverlay41Targets;

extern void func_overlay_041_F0001298_18885D0(Overlay41Input *,
                                                Overlay41Object *, s32);
extern void func_overlay_041_F0001464_188879C(Overlay41Input *,
                                                Overlay41Object *, s32);
extern void func_overlay_041_F00002AC_18875E4(Overlay41CurveNode *, f32,
                                                f32 *, f32 *, s32);
extern s16 func_overlay_041_F00007FC_1887B34(f32, s16, s16);
extern void func_8003EDEC(Overlay41Object *, s32);
extern s32 func_8000FAE0(f32, f32, f32);
extern f32 sqrtf(f32);
extern s32 func_8002A910(f32, f32);
extern f32 func_8002A878(f32, s32);

/* Workbench p4: structure-mismatch; 222 positional/225 raw words differ,
 * 638/638 instructions, first +0x80, frame exact -224. Levers: absolute-
 * constant audit and frameAmount declaration; both regressed; remains FP homes. */
#ifdef NON_MATCHING
void func_overlay_041_F0000854_1887B8C(Overlay41Input *input, f32 amount,
                                        s32 updateRate, s32 argument) {
    s32 iteration;
    s16 targetX;
    s16 targetY;
    s16 targetZ;
    f32 currentSpeed;
    f32 averageSpeed;
    f32 distance;
    f32 sampleX;
    f32 sampleY;
    f32 sampleZ;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 precision;
    f32 distanceTotal;
    f32 t;
    f32 blend;
    f32 precisionDelta;
    f32 probeT;
    f32 remaining;
    f32 probeSpeed;
    f32 elapsed;
    f32 frameAmount;
    Overlay41CurveNode *node;
    Overlay41CurveNode *next;
    Overlay41Object *object;

    if (input != 0 && input->object != 0 && !(amount < 0.0f)) {
        object = input->object;
        frameAmount = amount * 60.0f;
        func_overlay_041_F0001298_18885D0(input, object, argument);
        if ((input->flags & 4) || (gOverlay41Disabled < 0)) {
            func_overlay_041_F0001464_188879C(input, object, argument);
            func_8003EDEC(object, updateRate);
            return;
        }

        node = input->node;
        t = input->t;
        if (amount == 0.0f) {
            func_overlay_041_F00002AC_18875E4(node, t, &object->x, 0, 0);
            func_overlay_041_F00002AC_18875E4(node, t, &object->y, 0, 1);
            func_overlay_041_F00002AC_18875E4(node, t, &object->z, 0, 2);
            object->positionTag = func_8000FAE0(object->x, object->y, object->z);
        } else if (node->speed != 0.0f || node->next->speed != 0.0f) {
            precision = 0.01f;
            precisionDelta = 0.005f;
            remaining = amount;
            iteration = 0;
            currentSpeed = node->speed + (t * (node->next->speed - node->speed));
            object->velocityX = object->x;
            object->velocityY = object->y;
            object->velocityZ = object->z;
            distanceTotal = 0.0f;

            if (t != 1.0f || node->next->next != node->next) {
                do {
                    next = node->next;
                    if (t == 1.0f) {
                        node = next;
                        precision = 0.01f;
                        next = next->next;
                        precisionDelta = 0.005f;
                        t = 0.0f;
                        iteration = 0;
                    }

                    probeT = t + precision;
                    if (probeT > 1.0f) {
                        probeSpeed = next->speed;
                        sampleX = node->next->x;
                        probeT = 1.0f;
                        sampleY = node->next->y;
                        sampleZ = node->next->z;
                    } else {
                        probeSpeed = node->speed +
                                     (probeT * (next->speed - node->speed));
                        func_overlay_041_F00002AC_18875E4(node, probeT,
                                                           &sampleX, 0, 0);
                        func_overlay_041_F00002AC_18875E4(node, probeT,
                                                           &sampleY, 0, 1);
                        func_overlay_041_F00002AC_18875E4(node, probeT,
                                                           &sampleZ, 0, 2);
                    }

                    dx = sampleX - object->x;
                    dy = sampleY - object->y;
                    dz = sampleZ - object->z;
                    distance = sqrtf((dx * dx) + (dy * dy) + (dz * dz));
                    averageSpeed = (currentSpeed + probeSpeed) * 0.5f;
                    if (averageSpeed > 0.0f) {
                        elapsed = distance / averageSpeed;
                        remaining -= elapsed;
                        if (((-0.00001f < remaining) &&
                             (remaining < 0.00001f)) || iteration == 8) {
                            iteration = 9;
                            object->x = sampleX;
                            object->y = sampleY;
                            object->z = sampleZ;
                            object->positionTag =
                                func_8000FAE0(sampleX, sampleY, sampleZ);
                            t = probeT;
                            distanceTotal += distance;
                        } else if (iteration == 0 && remaining > 0.0f) {
                            object->x = sampleX;
                            object->y = sampleY;
                            object->z = sampleZ;
                            object->positionTag =
                                func_8000FAE0(sampleX, sampleY, sampleZ);
                            t = probeT;
                            currentSpeed = probeSpeed;
                            distanceTotal += distance;
                        } else {
                            if (remaining > 0.0f) {
                                precision += precisionDelta;
                            } else {
                                precision -= precisionDelta;
                            }
                            remaining += elapsed;
                            iteration++;
                            precisionDelta *= 0.5f;
                        }
                    } else {
                        iteration = 9;
                    }
                } while (iteration < 9 &&
                         (t != 1.0f || node->next->next != node->next));
            }

            object->velocityX = (object->x - object->velocityX) / frameAmount;
            object->velocityY = (object->y - object->velocityY) / frameAmount;
            object->velocityZ = (object->z - object->velocityZ) / frameAmount;
        }

        input->node = node;
        input->t = t;
        if (input->scaleTimer != 0) {
            if (updateRate < input->scaleTimer) {
                input->scaleTimer -= updateRate;
                input->scaleAmount += input->scaleRate * frameAmount;
            } else {
                input->scaleAmount += input->scaleRate * (f32)input->scaleTimer;
                input->scaleTimer = 0;
            }
        }

        func_overlay_041_F00002AC_18875E4(node, t, &dx, 0, 6);
        object->scale = object->extra->scale * input->scaleAmount * dx;
        if (object->scalePair != 0) {
            object->scalePair[0] = object->extra->scaleX * dx;
            object->scalePair[1] = object->extra->scaleY * dx;
        }

        targetX = object->angleX;
        targetY = object->angleY;
        targetZ = object->angleZ;
        switch (input->orientationMode) {
            case 1:
                dx = (f32)node->tangentX +
                     ((f32)(node->next->tangentX - node->tangentX) * t);
                dy = (f32)node->tangentY +
                     ((f32)(node->next->tangentY - node->tangentY) * t);
                dz = (f32)node->tangentZ +
                     ((f32)(node->next->tangentZ - node->tangentZ) * t);
                targetX += (s32)(dx * frameAmount);
                targetY += (s32)(dy * frameAmount);
                targetZ += (s32)(dz * frameAmount);
                break;
            case 2:
                func_overlay_041_F00002AC_18875E4(node, t, 0, &dx, 0);
                func_overlay_041_F00002AC_18875E4(node, t, 0, &dy, 1);
                func_overlay_041_F00002AC_18875E4(node, t, 0, &dz, 2);
                targetX = func_8002A910(dx, dz) - 0x8000;
                targetY = func_8002A910(dy,
                                        sqrtf((dx * dx) + (dy * dy) +
                                              (dz * dz)));
                func_overlay_041_F00002AC_18875E4(node, t, &dz, 0, 5);
                targetZ = (s32)dz;
                break;
            case 3:
                if (gOverlay41Targets[input->targetIndex] != 0 &&
                    gOverlay41Targets[input->targetIndex]->object != 0) {
                    Overlay41Object *targetObject;
                    targetObject = gOverlay41Targets[input->targetIndex]->object;
                    dx = targetObject->x - object->x;
                    dy = targetObject->y - object->y;
                    dz = targetObject->z - object->z;
                    targetX = func_8002A910(dx, dz) - 0x8000;
                    targetY = func_8002A910(dy,
                                            sqrtf((dx * dx) + (dy * dy) +
                                                  (dz * dz)));
                    targetZ = 0;
                }
                break;
            default:
                func_overlay_041_F00002AC_18875E4(node, t, &dx, 0, 3);
                func_overlay_041_F00002AC_18875E4(node, t, &dy, 0, 4);
                func_overlay_041_F00002AC_18875E4(node, t, &dz, 0, 5);
                targetX = (s32)dx;
                targetY = (s32)dy;
                targetZ = (s32)dz;
                break;
        }

        if (input->smoothing != 1.0f) {
            blend = 1.0f - func_8002A878(1.0f - input->smoothing, updateRate);
            t = blend;
            object->angleX = func_overlay_041_F00007FC_1887B34(
                t, object->angleX, targetX);
            object->angleY = func_overlay_041_F00007FC_1887B34(
                t, object->angleY, targetY);
            object->angleZ = func_overlay_041_F00007FC_1887B34(
                t, object->angleZ, targetZ);
        } else {
            object->angleX = targetX;
            object->angleY = targetY;
            object->angleZ = targetZ;
        }

        func_overlay_041_F0001464_188879C(input, object, updateRate);
        func_8003EDEC(object, updateRate);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o041/overlay41UpdateCurveObject/func_overlay_041_F0000854_1887B8C.s")
#endif
