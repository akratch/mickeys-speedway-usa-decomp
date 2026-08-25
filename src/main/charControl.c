/*
 * Character and camera control -- ROM 0x1C790-0x20020
 * (VRAM 0x8001BB90-0x8001F420).
 *
 * The yaml boundaries were originally splat's aligned file-boundary
 * candidates. The content now supports the TU assignment independently: the
 * first routines follow JFG's camera-control cluster, exact skeleton anchors
 * identify func_8001C2D4 and controlSetPlayerSetup inside the block, the tail
 * is the same player-setup set/get/clear sequence, and the next yaml block
 * starts with a tier-A JFG models.c function. See docs/modules.md section 3.4.
 *
 * PROVENANCE -- Jet Force Gemini's public decomp src/charControl.c,
 * src/charControl.h, built charControl.c object, public symbol map, and
 * asm/nonmatchings/charControl filenames were consulted to identify the
 * translation unit and obtain comparison leads. Names not already supported
 * by tier-A evidence remain comments in docs/modules.md and are not adopted
 * here. Any future body adapted from JFG must carry its own PROVENANCE note
 * before that body and must be proved against Mickey's bytes.
 *
 * Flags: -O2 -mips2 -32 -Wab,-r4300_mul, measured on func_8001F09C.
 */

#include "PR/ultratypes.h"
#include "game/charControl.h"

extern u8 D_80079BF8;
extern u16 D_80079A0C[];
extern u16 D_80079A20[][4];
extern ControlVector3 D_800799EC;
extern ControlVector3 D_800799FC;
extern u16 D_8007BF1C;
extern f32 D_80081894;
extern f32 D_80081898;
extern s32 D_80079BCC;
extern s32 D_8007C1A0;
extern f32 D_80079BD4[];
extern CameraTrackedObject *D_800CB308[];
extern CameraOverrideSlot D_800CB368[];
extern CameraOverride D_800CB380[];
extern s16 D_800CB470;
extern s16 D_800CB472;
extern s16 D_800CB474;
extern s16 D_800CB476;
extern u8 *D_800CB300;

void pointListRPY(s32 count, s16 *rotation, f32 *input, f32 *output);
void func_8001EFFC(ControlTransform *transform, ControlPlayer *player, f32 *output);
f32 func_8002A8BC(s32 angle);
f32 func_8002A8C0(s32 angle);
s16 Arctanf(f32 x, f32 y);
void mathOneFloatRPY(ControlTransform *transform, f32 *output);
s32 mathRnd(s32 minimum, s32 maximum);
ControlSpawned *func_8000590C(ControlSpawnPacket *packet, s32 mode);
void func_800031E8(void *handle);
void func_80002FE0(s32 id, f32 x, f32 y, f32 z, s32 priority, void **handle);
void func_8001D690(ControlActor *actor, ControlPlayer *player);
void func_80006EA0(void *handle);
s32 func_8000FAE0(f32 x, f32 y, f32 z);
void func_8001C4C0(ControlActor *actor, ControlPlayerInitState *state, s32 mode);
s32 TrapDanglingJump();
void func_8001BBB4(ControlActor *actor, ControlPlayer *player, f32 arg2);
void camSetNo(s8 playerIndex, s32 cameraIndex, u8 **cameraState);
u8 *camGetListPtr(void);
s32 mainGetNumberOfCameras(void);
s32 func_800299E8(s32 minimum, s32 maximum);
ControlActor **func_8000572C(s32 *start, s32 *end);
s32 func_8005776C(f32 x, f32 y, f32 z, f32 radius, s32 mode, ControlActor **hitActor);
void func_800282C8(void);
u32 joyGetButtons(s32 playerIndex);
u32 joyGetPressed(s32 playerIndex);
u32 joyGetReleased(s32 playerIndex);
u32 joyGetStickX(s32 playerIndex);
u32 joyGetAbsX(s32 playerIndex);
u32 joyGetStickY(s32 playerIndex);
u32 joyGetAbsY(s32 playerIndex);
void func_800291D8(s32 arg0);
s32 func_800291FC(void);
void rumbleStart(s32 playerIndex, s32 strength, f32 duration);

f32 func_8001BB90(s32 cameraIndex) {
    return D_800CB380[cameraIndex].blend;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/func_8001BBB4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/func_8001BE0C.s")
void func_8001C054(CameraTrackedObject *value) {
    if (D_80079BCC < 24) {
        D_800CB308[D_80079BCC] = value;
        D_80079BCC++;
    }
}
void func_8001C088(CameraTrackedObject *value) {
    s32 index;
    s32 foundIndex;

    index = 0;
    foundIndex = -1;
    if (D_80079BCC > 0) {
        do {
            if (D_800CB308[index] == value) {
                foundIndex = index;
            }
            index++;
        } while (index < D_80079BCC);
    }
    if (foundIndex != -1) {
        for (index = foundIndex; index < D_80079BCC - 1; index++) {
            D_800CB308[index] = D_800CB308[index + 1];
        }
        D_80079BCC--;
    }
}
/* Workbench: mixed frame/allocation residual, 106/108 instructions; first mismatch +0x0.
 * Tried register-qualified parameters/locals, named aliases, and empty-read pool levers.
 * The second FP callee-save web remains absent, leaving a 0x10 rather than 0x18 frame. */
#ifdef NON_MATCHING
void func_8001C114(s32 slotIndex, f32 x, f32 y, f32 z) {
    CameraOverrideSlot *slot;
    CameraTrackedObject *object;
    CameraTrackedObject **current;
    CameraBounds *bounds;
    f32 deltaX;
    f32 deltaZ;
    f32 trackedRadiusSquared;
    f32 radiusSquared;
    s32 index;

    if (slotIndex >= 0 && slotIndex < 4) {
        slot = &D_800CB368[slotIndex];
        object = slot->object;
        if (object != 0) {
            bounds = slot->bounds;
            if (bounds != 0) {
                deltaX = object->x - x;
                deltaZ = object->z - z;
                trackedRadiusSquared = bounds->trackedRadius * bounds->trackedRadius;
                if (trackedRadiusSquared <
                    ((deltaX * deltaX) + (deltaZ * deltaZ))) {
                    slot->object = 0;
                    object = 0;
                } else if ((bounds->flags & 0x8000) &&
                           ((y < bounds->trackedUpper) || (bounds->trackedLower < y))) {
                    slot->object = 0;
                    object = 0;
                }
            }
        }
        if (object == 0) {
            current = D_800CB308;
            index = 0;
            if (D_80079BCC > 0) {
                do {
                    object = *current;
                    bounds = object->bounds;
                    radiusSquared = bounds->radius * bounds->radius;
                    deltaX = object->x - x;
                    deltaZ = object->z - z;
                    if (((deltaX * deltaX) + (deltaZ * deltaZ)) <
                        radiusSquared) {
                        slot->object = object;
                        slot->bounds = bounds;
                        if ((bounds->flags & 0x4000) &&
                            ((y < bounds->upper) || (bounds->lower < y))) {
                            slot->object = 0;
                        }
                    }
                    index++;
                    current++;
                } while (index < D_80079BCC);
            }
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/func_8001C114.s")
#endif
void func_8001C2C4(void) {
}
void func_8001C2CC(void) {
}
void func_8001C2D4(u8 *start, u8 *end) {
    u8 *current = start;

    if (start < end) {
        do {
            *current++ = 0;
        } while (current != end);
    }
}
/*
 * PROVENANCE -- JFG's charControl symbols and assembly supplied the
 * controlPlayerReInit name/role. This Mickey-specific save, clear, initialize,
 * and restore body is independently reconstructed from Mickey's code.
 */
void controlPlayerReInit(ControlActor *actor, f32 x, f32 y, f32 z, s16 arg4, s16 arg5, s16 arg6) {
    ControlPlayer *player;
    s32 saved192;
    s32 saved1A8;
    s32 saved3BA;
    s32 saved45C;
    s32 saved45D;
    ControlPlayerInitState stateStorage;
    ControlPlayerInitState *state;

    player = actor->player;
    state = &stateStorage;
    state->playerIndex = player->playerIndex;
    state->unk11 = player->unk1;
    state->arg4 = arg4;
    state->arg5 = arg5;
    state->arg6 = arg6;
    saved192 = player->unk192;
    saved1A8 = player->flags1A8;
    saved3BA = player->unk3BA;
    saved45C = player->unk45C;
    saved45D = player->unk45D;
    actor->x = x;
    actor->y = y;
    actor->flags &= ~0x400;
    actor->z = z;
    actor->velocityX = 0.0f;
    actor->velocityY = 0.0f;
    actor->velocityZ = 0.0f;
    actor->positionTag = func_8000FAE0(actor->x, actor->y, actor->z);
    actor->alpha = 0xFF;
    actor->unk80 = 0;
    if (player->unkD0 != 0) {
        func_80006EA0(player->unkD0);
    }
    if (player->unkD4 != 0) {
        func_80006EA0(player->unkD4);
    }
    if (player->unkD8 != 0) {
        func_80006EA0(player->unkD8);
    }
    func_8001C2D4((u8 *) player, (u8 *) player + 0xA4);
    func_8001C2D4((u8 *) player + 0xC8, (u8 *) player + 0x134);
    func_8001C2D4((u8 *) player + 0x144, (u8 *) player + 0x19A);
    func_8001C2D4((u8 *) player + 0x1A4, (u8 *) player + 0x34C);
    func_8001C2D4((u8 *) player + 0x3E4, (u8 *) player + 0x400);
    func_8001C4C0(actor, state, 0);
    player->unk11C[0] = -2.0f;
    player->unk11C[1] = -2.0f;
    player->unk11C[2] = -2.0f;
    player->unk11C[3] = -2.0f;
    player->unk192 = saved192;
    player->flags1A8 = saved1A8;
    player->unk3BA = saved3BA;
    player->unk45C = saved45C;
    player->unk45D = saved45D;
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/func_8001C4C0.s")
void func_8001CB0C(ControlTransform *transform, ControlPlayer *player) {
    player->unk2BC = 1;
    if (D_8007BF1C & 8) {
        player->unk2B8 = &D_800799FC;
    } else {
        player->unk2B8 = &D_800799EC;
    }
    player->unk33C = 0;
    player->unk340 = 0;
    player->unk2C0[0] = player->unk2B8->x;
    player->unk2C0[1] = player->unk2B8->y;
    player->unk2C0[2] = player->unk2B8->z;
    func_8001EFFC(transform, player, &player->unk2C0[12]);
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/func_8001CB84.s")
/*
 * Workbench: mixed(structural:1, register:8), 40 words, +1 instruction, first mismatch +0xE0.
 * Levers tried: structure/type/AST association via volatile storage, old-style prototype, comma sequencing, and a result local.
 * Remains: global-address materialisation adds one instruction before the call and shifts the later integer temp web.
 */
#ifdef NON_MATCHING
void func_8001D2A0(ControlActor *actor, s32 arg1) {
    ControlPlayer *player;
    s32 cameraIndex;

    player = actor->player;
    player->unk43C = actor->rotationX;
    player->unk43E = actor->rotationY;
    player->unk440 = actor->rotationZ;
    player->unk444 = actor->unk8;
    player->unk448 = actor->x;
    player->unk44C = actor->y;
    player->unk450 = actor->z;
    if (player->unk158 != 0) {
        player->unk43C += player->unk160;
        player->unk43E += player->unk164;
        player->unk440 += player->unk162;
        player->unk44C += player->unk154 + player->unk14C;
    }
    if (!(player->flags1A8 & 1)) {
        TrapDanglingJump(actor, player, arg1);
    }
    if (player->unkD4 != 0) {
        TrapDanglingJump(player->unkD4, arg1);
    }
    D_800CB300 = camGetListPtr();
    cameraIndex = mainGetNumberOfCameras() - 1;
    if (player->playerIndex < cameraIndex) {
        cameraIndex = player->playerIndex;
    }
    D_800CB300 += cameraIndex * 0x54;
    camSetNo(player->playerIndex, cameraIndex, &D_800CB300);
    if ((player->unk190 != 0) || (player->unk3FA == 0)) {
        func_8001BBB4(actor, player, (f32) arg1);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/func_8001D2A0.s")
#endif
void func_8001D41C(ControlActor *actor, ControlPlayer *player, s32 updateRate) {
    ControlPlayerActions *actions;
    ControlPlayerAction action;
    s32 effectIndex;

    if (player->unk19C > 0) {
        player->unk19C -= updateRate;
        if (player->unk19C <= 0) {
            player->unk19C = 0;
            if ((D_8007C1A0 == 1) && !(player->flags1A8 & 1)) {
                effectIndex = player->unk19A;
                if ((effectIndex >= 2) && (effectIndex < 10)) {
                    TrapDanglingJump(effectIndex + 30);
                }
            }
        }
    }

    actions = player->actions;
    if ((actions == 0) || (player->unk19C != 0) ||
        (TrapDanglingJump(player->unkD4) != 0)) {
        if (player->controlDkeys & 0x2000) {
            if ((player->unk1 >= 0) && (player->unk1 < 10)) {
                if (D_8007BF1C & 4) {
                    if (player->unkA4 != 0) {
                        func_800031E8(player->unkA4);
                    }
                    func_80002FE0(
                        D_80079A20[player->unk1][func_800299E8(0, 3)],
                        actor->x, actor->y, actor->z, 4, &player->unkA4);
                    return;
                }
                if (player->unkA8 != 0) {
                    func_800031E8(player->unkA8);
                }
                func_80002FE0(D_80079A0C[player->unk1], actor->x, actor->y,
                              actor->z, 4, &player->unkA8);
            }
        }
    } else if (player->controlDkeys & 0x2000) {
        action = actions->positive;
        if ((action != 0) && (player->controlYjoy >= 65)) {
            action(actor);
            return;
        }
        action = actions->negative;
        if ((action != 0) && (player->controlYjoy < -64)) {
            action(actor);
            return;
        }
        action = actions->fallback;
        if (action != 0) {
            action(actor);
        }
    }
}
void controlFrozen(ControlActor *actor, ControlPlayer *player) {
    if (func_800291FC() == 1) {
        func_800291D8(10);
    }
    if (joyGetPressed(player->playerIndex) & 0xF00F) {
        func_8001D690(actor, player);
    }
}
/*
 * PROVENANCE -- JFG's charControl symbols and assembly supplied the
 * controlRestartPlayer name/role. This Mickey-specific respawn-point search
 * and reinitialization body is independently reconstructed from Mickey's code.
 */
void func_8001D690(ControlActor *actor, ControlPlayer *player) {
    s32 start;
    s32 end;
    ControlActor **objects;
    ControlActor *current;
    s32 maxIndex;
    s32 playerCount;
    f32 radius;
    s32 mode;
    s32 candidateIndex;
    ControlActor *hitActor;
    ControlActor *candidates[8];
    s32 count;
    s32 hit;

    playerCount = func_800291FC();
    if (playerCount >= 2) {
        objects = func_8000572C(&start, &end);
        count = 0;
        radius = 32.0f;
        mode = 0;
        if (start < end) {
            do {
                current = objects[start++];
                if (current->kind == 5) {
                    hit = func_8005776C(current->x, current->y, current->z,
                                       radius, mode, &hitActor);
                    if ((hit == 0) || ((hit == 1) && (actor == hitActor))) {
                        candidates[count++] = current;
                    }
                }
            } while (start < end);
        }
        if (count == 0) {
            current = actor;
        } else if (count == 1) {
            current = candidates[0];
        } else {
            maxIndex = count - 1;
            candidateIndex = mathRnd(0, maxIndex);
            current = candidates[candidateIndex];
        }
        controlPlayerReInit(actor, current->x, current->y, current->z,
                            current->rotationX, current->rotationY,
                            current->rotationZ);
    } else {
        func_800282C8();
    }
}
/* PROVENANCE -- adapted from JFG's src/charControl.c dAngle. */
s16 dAngle(s16 arg0, s16 arg1, f32 arg2) {
    s32 temp_t1;
    s32 var_v1;

    var_v1 = (arg1 - arg0) & 0xFFFF;
    temp_t1 = (arg0 - arg1) & 0xFFFF;
    if (temp_t1 < var_v1) {
        var_v1 = -temp_t1;
    }
    return (s16) (arg0 + (s32) ((f32) var_v1 * arg2));
}
/* PROVENANCE -- adapted from JFG's charControl controlMakeV implementation. */
#ifdef NON_MATCHING
f32 func_8001D880(f32 arg0, f32 arg1, f32 *table, f32 divisor) {
    f32 base;
    f32 value;
    s32 index;
    f32 *entry;

    arg1 *= 10.0f;
    index = (s32) arg1;
    entry = table + index;
    base = entry[0];
    value = ((entry[1] - base) * (arg1 - (f32) index)) + base;
    arg0 *= 10.0f;
    index = (s32) arg0;
    entry = table + index;
    base = entry[0];
    return (value - (base + ((entry[1] - base) * (arg0 - (f32) index)))) / divisor;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/func_8001D880.s")
#endif
/* PROVENANCE -- adapted from JFG's src/charControl.c controlFSUvels. */
void controlFSUvels(s16 *rotation, ControlPlayer *player) {
    s16 sp18[3];

    sp18[0] = rotation[0];
    sp18[1] = rotation[1];
    sp18[2] = 0;
    pointListRPY(3, sp18, D_80079BD4, player->unk14);
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/func_8001D960.s")
void func_8001DCD0(s16 rotation, ControlVector3 *vector, s16 *pitch, s16 *yaw) {
    f32 cosine;
    f32 pitchX;
    s32 angle;
    f32 y;
    f32 transformedX;

    angle = -rotation;
    cosine = func_8002A8C0(angle);
    transformedX = func_8002A8BC(angle);
    y = vector->y;
    pitchX = (vector->z * cosine) + (vector->x * transformedX);
    transformedX = (vector->z * transformedX) - (vector->x * cosine);
    *pitch = Arctanf(-pitchX, y);
    *yaw = Arctanf(transformedX, y);
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/func_8001DD70.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/func_8001E5C4.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/func_8001EC44.s")
void func_8001EFFC(ControlTransform *transform, ControlPlayer *player, f32 *output) {
    f32 *current;
    s32 index;

    pointListRPY(player->unk2BC, (s16 *) transform, player->unk2C0, output);
    current = output;
    index = 0;
    if (player->unk2BC > 0) {
        do {
            current[0] += transform->x;
            current[1] += transform->y;
            current[2] += transform->z;
            current += 3;
            index++;
        } while (index < player->unk2BC);
    }
}
void func_8001F09C(ControlPlayer *player, s32 updateRate) {
    f32 rate;

    rate = (f32) updateRate;
    if (player->unk50 < player->unk54) {
        player->unk50 += ((player->unk54 - player->unk50) * 0.125f * rate) + D_80081894;
        if (player->unk54 <= player->unk50) {
            player->unk50 = player->unk54;
        }
    } else {
        player->unk50 += ((player->unk54 - player->unk50) * 0.125f * rate) - D_80081898;
        if (player->unk50 <= player->unk54) {
            player->unk50 = player->unk54;
        }
    }
}
void func_8001F14C(ControlTransform *transform, ControlCeilingContext *context) {
    register ControlSpawned *spawned;
    ControlSpawnPacket packet;
    f32 offset[3];
    f32 x;
    f32 y;
    f32 z;

    offset[0] = 0.0f;
    offset[1] = 0.0f;
    offset[2] = 10.0f;
    mathOneFloatRPY(transform, offset);
    x = offset[0] + transform->x;
    y = context->height;
    z = offset[2] + transform->z;
    packet.kind = 0x157;
    packet.mode = 0xC;
    packet.flags = 0;
    packet.x = (s16) x;
    packet.y = (s16) y;
    packet.z = (s16) z;
    packet.unkA = mathRnd(-0x7FFF, 0x7FFF);
    spawned = func_8000590C(&packet, 1);
    if (spawned != 0) {
        spawned->unk3C = 0;
    }
    if (context->handle != 0) {
        func_800031E8(context->handle);
    }
    func_80002FE0(0x329, x, y, z, 4, &context->handle);
}
/*
 * PROVENANCE -- JFG's src/charControl.c supplied the controlDisableJoypad
 * name/role. Mickey's two-argument field store independently determines this
 * per-player body and differs from JFG's one-argument global implementation.
 */
void controlDisableJoypad(ControlPlayer *player, s32 disabled) {
    player->joypadDisabled = disabled;
}
/* PROVENANCE -- adapted from JFG's src/charControl.c controlReadJoypad. */
void controlReadJoypad(ControlPlayer *player, s32 playerIndex) {
    if ((playerIndex >= 0) && (playerIndex < 4) && (player->joypadDisabled == 0)) {
        player->controlXjoy = joyGetStickX(playerIndex);
        player->controlAbsXjoy = joyGetAbsX(playerIndex);
        player->controlYjoy = joyGetStickY(playerIndex);
        player->controlAbsYjoy = joyGetAbsY(playerIndex);
        player->controlKeys = joyGetButtons(playerIndex);
        player->controlDkeys = joyGetPressed(playerIndex);
        player->controlReleasedKeys = joyGetReleased(playerIndex);
    } else {
        player->controlXjoy = 0;
        player->controlAbsXjoy = 0;
        player->controlYjoy = 0;
        player->controlAbsYjoy = 0;
        player->controlKeys = 0;
        player->controlDkeys = 0;
        player->controlReleasedKeys = 0;
    }
}
/*
 * PROVENANCE -- JFG's charControl symbols supplied the controlSetRumble
 * name/role. Mickey's smaller wrapper independently determines this body.
 */
void controlSetRumble(ControlPlayer *player, s32 strength, f32 duration) {
    if ((player->unk191 == 0) && !(player->flags1A8 & 1)) {
        rumbleStart(player->playerIndex, strength, duration);
    }
}
void func_8001F364(void) {
}
void controlSetPlayerSetup(s16 arg0, s16 arg1, s16 arg2, s16 arg3) {
    D_800CB470 = arg0;
    D_800CB472 = arg1;
    D_800CB474 = arg2;
    D_800CB476 = arg3;
    D_80079BF8 = 1;
}
/*
 * PROVENANCE -- JFG's charControl symbols supplied the controlGetPlayerSetup
 * name/role. This body is reconstructed from Mickey's setup-state accesses.
 */
s32 controlGetPlayerSetup(s16 *arg0, s16 *arg1, s16 *arg2, s16 *arg3) {
    if (D_80079BF8 != 0) {
        *arg0 = D_800CB470;
        *arg1 = D_800CB472;
        *arg2 = D_800CB474;
        *arg3 = D_800CB476;
        D_80079BF8 = 0;
        return 1;
    }
    return 0;
}

/* PROVENANCE -- adapted from JFG's src/charControl.c controlClearPlayerSetup. */
void controlClearPlayerSetup(void) {
    D_80079BF8 = 0;
}
