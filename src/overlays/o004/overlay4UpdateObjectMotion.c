#include "PR/ultratypes.h"

typedef struct Overlay4MotionState {
    f32 increment;
    s16 targetAngle;
    s16 phase;
    u8 timer;
    u8 trigger;
} Overlay4MotionState;

typedef struct Overlay4Config {
    u8 pad00[0xA];
    s8 threshold;
    u8 pad0B;
    u8 mode;
    u8 timerMinimum;
    u8 timerMaximum;
    u8 spawnMinimum;
    u8 spawnMaximum;
    u8 spawnChance;
    u8 baseAngle;
    u8 phaseSpeed;
    s8 amplitude;
} Overlay4Config;

typedef struct Overlay4Position {
    f32 x;
    f32 y;
    f32 z;
} Overlay4Position;

typedef struct Overlay4PositionOwner {
    u8 pad00[0x40];
    Overlay4Position *position;
} Overlay4PositionOwner;

typedef struct Overlay4SpawnState {
    s32 value;
    s16 angle;
    u8 active;
    u8 field07;
    u8 field08;
    u8 pad09;
    u8 field0A;
} Overlay4SpawnState;

typedef struct Overlay4Spawned {
    u8 pad00[0x3C];
    s32 config;
    u8 pad40[0x24];
    Overlay4SpawnState *state;
} Overlay4Spawned;

typedef struct Overlay4Object {
    s16 angle;
    s16 outputAngle;
    u8 pad04[0x36];
    s8 positionIndex;
    u8 pad3B;
    Overlay4Config *config;
    u8 pad40[0x24];
    Overlay4MotionState *motion;
    Overlay4PositionOwner **positionOwners;
} Overlay4Object;

typedef struct Overlay4SpawnPacket {
    s16 kind;
    u8 mode;
    u8 flags;
    s16 x;
    s16 y;
    s16 z;
} Overlay4SpawnPacket;

extern void func_8005ABA8(Overlay4Object *object, f32 scale, f32 updateRate);
extern s32 mathRnd(s32 minimum, s32 maximum);
extern s32 mathDiffAngle(s16 current, s16 target);
extern f32 func_80029274(s32 delta, f32 current, f32 amount);
extern f32 func_8002A8C0(s16 angle);
extern s32 func_80004590(s32 maximum);
extern Overlay4Spawned *func_8000590C(Overlay4SpawnPacket *packet, s32 mode);
extern void func_overlay_036_F00007B0(Overlay4Spawned *spawned, s16 angle,
                                      s16 outputAngle, f32 value);

#ifdef NON_MATCHING
void overlay4UpdateObjectMotion(Overlay4Object *object, s32 updateRate) {
    Overlay4MotionState *motion;
    Overlay4Config *config;
    Overlay4PositionOwner *positionOwner;
    Overlay4Spawned *spawned;
    Overlay4SpawnState *spawnState;
    Overlay4SpawnPacket packet;
    s32 delta;
    u8 timer;

    motion = object->motion;
    config = object->config;
    func_8005ABA8(object, 0.1f, (f32)updateRate);

    switch (config->mode) {
    case 0:
        timer = motion->timer;
        if (updateRate >= timer) {
            motion->trigger = 1;
            motion->timer =
                (u8)((f32)mathRnd(config->timerMinimum,
                                         config->timerMaximum) * 6.0f);
        } else {
            motion->timer = timer - updateRate;
        }
        break;
    case 1:
        delta = (s16)mathDiffAngle(object->angle, motion->targetAngle);
        if (delta <= config->threshold && delta >= -config->threshold) {
            motion->targetAngle = mathRnd(-0x8000, 0x7FFF);
            motion->trigger = 1;
        } else {
            delta = mathDiffAngle(object->angle, motion->targetAngle);
            motion->increment +=
                func_80029274(delta, motion->increment,
                              (f32)config->threshold);
        }
        break;
    }

    object->angle = (s16)((f32)object->angle + motion->increment);
    motion->phase += updateRate * config->phaseSpeed * 10;
    object->outputAngle =
        (s16)((func_8002A8C0(motion->phase) * (f32)config->amplitude +
               (f32)config->baseAngle) *
              256.0f);

    if (motion->trigger != 0) {
        motion->trigger = 0;
        if (func_80004590(0x21) < config->spawnChance) {
            positionOwner = object->positionOwners[object->positionIndex];
            packet.kind = 0x95;
            packet.mode = 10;
            packet.flags = 0;
            packet.x = (s16)positionOwner->position->x;
            packet.y = (s16)(positionOwner->position->y - 30.0f);
            packet.z = (s16)positionOwner->position->z;
            spawned = func_8000590C(&packet, 1);
            if (spawned != 0) {
                spawned->config = 0;
                func_overlay_036_F00007B0(
                    spawned, object->angle,
                    (s16)(object->outputAngle + 0x1DDD),
                    (f32)mathRnd(config->spawnMinimum,
                                       config->spawnMaximum));
                spawnState = spawned->state;
                spawnState->angle = 0;
                spawnState->value = 0;
                spawnState->active = 1;
                spawnState->field07 = 0;
                spawnState->field08 = 0;
                spawnState->field0A = 0x80;
            }
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o004/overlay4UpdateObjectMotion/func_overlay_004_F0000138_185A7B0.s")
#endif
