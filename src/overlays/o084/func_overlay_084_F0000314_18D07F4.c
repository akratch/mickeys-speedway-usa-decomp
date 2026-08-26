#include "PR/ultratypes.h"

typedef struct Overlay84UpdateChoice {
    u8 pad00[0x12];
    u8 first;
    u8 second;
} Overlay84UpdateChoice;

typedef struct Overlay84UpdateNode {
    s16 angle;
    u8 pad02[0x0A];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x24];
    Overlay84UpdateChoice *choice;
} Overlay84UpdateNode;

typedef struct Overlay84UpdateState {
    u8 initialized;
    s8 current;
    s8 status;
    s8 actionTimer;
    s8 blendTimer;
    s8 action;
    s8 inputLatch;
    s8 inputTimer;
    u8 pad08;
    u8 mode;
    u8 pad0A[4];
    s16 angleOffset;
    s16 targetTilt;
    s16 tilt;
    s16 angle;
    s16 targetAngle;
    u32 flags;
    u8 pad1C[4];
    union {
        s32 word;
        struct {
            s16 upper;
            s16 base;
        } half;
    } outputAngle;
    f32 targetHeight;
    f32 height;
    f32 blend;
    f32 x;
    f32 y;
    f32 z;
    s32 first;
    s32 second;
    Overlay84UpdateNode *nodes[32];
    u8 padC4[2];
    u8 marked;
} Overlay84UpdateState;

typedef struct Overlay84UpdateScale {
    u8 pad00[0x0A];
    s16 radius;
} Overlay84UpdateScale;

typedef struct Overlay84UpdateObject {
    u8 pad00[0x0C];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x24];
    Overlay84UpdateScale *volatile scale;
} Overlay84UpdateObject;

typedef struct Overlay84Output {
    s16 angle;
    s16 tilt;
    s16 roll;
    u8 pad06[6];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x26];
    s16 verticalAngle;
} Overlay84Output;

extern u32 gOverlay84InputFlags;
extern s16 gOverlay84InputAxis;
extern f32 gOverlay84BlendStep;

extern void overlay84EmitAction();
extern s16 overlay84AngleDifference(s16, s16);
extern f32 overlay84Sin(s16);
extern f32 overlay84Cos(s16);
extern void overlay84SelectOutput(s32);
extern Overlay84Output *overlay84GetOutput(void);
extern s16 overlay84VerticalAngle(f32, f32, f32);
extern s32 overlay84Atan2(f32, f32);
extern s16 overlay84BlendAngle(s16, s16, f32);

/* Workbench p5: structure-mismatch; 468/464 candidate/target instructions, 449 words from +0x3C.
 * Lever: constant-audit plus node-assignment and scale-scope order; both fresh guard forms regressed.
 * Remains: four extra instructions, relocation layout, and register/FP cascades after the first branch. */
#ifdef NON_MATCHING
void func_overlay_084_F0000314_18D07F4(Overlay84UpdateObject *object,
                                       Overlay84UpdateState *state,
                                       s32 updateRate) {
    Overlay84UpdateNode *node;
    Overlay84UpdateChoice *choice;
    Overlay84Output *output;
    s32 i;
    s32 angleStep;
    s32 angleFlag;
    s32 angleAdjust;
    register s16 currentAngle;
    register s16 targetAngle;
    f32 baseX;
    f32 baseY;
    f32 baseZ;
    f32 targetX;
    f32 targetY;
    f32 targetZ;
    Overlay84UpdateScale *scale;

    node = state->nodes[state->current];
    scale = object->scale;
    if (node != 0) {
        if (state->inputTimer > 0) {
            state->inputTimer -= updateRate;
        } else if (state->inputLatch == 0 &&
                   (gOverlay84InputFlags & 0x9000) != 0 &&
                   state->marked == 0) {
            overlay84EmitAction(0xC, 0);
            state->inputLatch = 1;
        }

        if (state->inputLatch == 0) {
            if (state->actionTimer > 0) {
                state->actionTimer -= updateRate;
            } else if (gOverlay84InputAxis < -0x10 && state->marked == 0) {
                overlay84EmitAction(1);
                state->actionTimer = 0x14;
                state->blendTimer = 0xA;
                state->action = 0;
                state->blend = 0.0f;
                overlay84EmitAction(0xF, 0);
            } else if (gOverlay84InputAxis >= 0x11 && state->marked == 0) {
                overlay84EmitAction(0);
                state->actionTimer = 0x14;
                state->blendTimer = 0xA;
                state->action = 0;
                state->blend = 0.0f;
                overlay84EmitAction(0xF, 0);
            }
        }

        currentAngle = state->angle;
        targetAngle = state->targetAngle;
        angleFlag = state->flags & 1;
        if (angleFlag != 0) {
            if (angleFlag == 1) {
                angleAdjust = 0xFFFF0000;
                if (currentAngle < targetAngle) {
                    goto adjust_target_angle;
                }
            }
        } else {
            angleAdjust = 0x10000;
            if (targetAngle < currentAngle) {
adjust_target_angle:
                targetAngle += angleAdjust;
            }
        }

        angleStep = 0;
        for (i = 0; i < updateRate; i++) {
            state->tilt += (s16)((state->targetTilt - state->tilt) * 0.125f);
            state->height += (state->targetHeight - state->height) * 0.125f;
            angleStep =
                overlay84AngleDifference(currentAngle, targetAngle) >> 5;
            if (angleStep == 0) {
                currentAngle = targetAngle;
            } else {
                currentAngle += angleStep;
            }
        }

        if (angleStep >= -0x1F && angleStep < 0x20) {
            choice = node->choice;
            state->first = choice->first;
            state->second = choice->second;
        } else {
            state->first = 0xFF;
            state->second = 0xFF;
        }
        state->angle = currentAngle;

        if (state->blendTimer > 0) {
            state->blendTimer -= updateRate;
        } else {
            state->blend += gOverlay84BlendStep * updateRate;
            if (state->blend > 1.0f) {
                state->blend = 1.0f;
            }
        }

        baseX = overlay84Sin(state->angle) * scale->radius + object->x;
        baseY = state->height + object->y;
        baseZ = overlay84Cos(state->angle) * scale->radius + object->z;
        targetX = baseX + state->blend * (node->x - baseX);
        targetY = baseY + state->blend * (node->y - baseY);
        targetZ = baseZ + state->blend * (node->z - baseZ);

        for (i = 0; i < updateRate; i++) {
            state->x += (targetX - state->x) * 0.125f;
            state->y += (targetY - state->y) * 0.125f;
            state->z += (targetZ - state->z) * 0.125f;
        }

        overlay84SelectOutput(0);
        output = overlay84GetOutput();
        output->x = state->x;
        output->y = state->y;
        output->z = state->z;
        output->verticalAngle =
            overlay84VerticalAngle(output->x, output->y, output->z);
        angleStep = -overlay84Atan2(object->x - output->x,
                                    object->z - output->z);
        angleStep = overlay84BlendAngle(angleStep, 0x8000 - node->angle,
                                        state->blend);
        state->outputAngle.word = overlay84BlendAngle(
            state->outputAngle.half.base, angleStep,
            1.0f - (f32)state->actionTimer / 20.0f);
        output->angle = state->outputAngle.word;
        output->roll = 0;
        output->tilt = state->tilt + state->angleOffset;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o084/func_overlay_084_F0000314_18D07F4/func_overlay_084_F0000314_18D07F4.s")
#endif
