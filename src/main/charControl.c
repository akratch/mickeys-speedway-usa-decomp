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
extern ControlVector3 D_800799EC;
extern ControlVector3 D_800799FC;
extern u16 D_8007BF1C;
extern f32 D_80081894;
extern f32 D_80081898;
extern s32 D_80079BCC;
extern f32 D_80079BD4[];
extern CameraTrackedObject *D_800CB308[];
extern CameraOverrideSlot D_800CB368[];
extern CameraOverride D_800CB380[];
extern s16 D_800CB470;
extern s16 D_800CB472;
extern s16 D_800CB474;
extern s16 D_800CB476;

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
void func_8001D690(s32 arg0, ControlPlayer *player);
u32 func_800254FC(s32 playerIndex);
u32 func_8002554C(s32 playerIndex);
u32 func_80025594(s32 playerIndex);
u32 func_800255DC(s32 playerIndex);
u32 func_80025634(s32 playerIndex);
u32 func_8002565C(s32 playerIndex);
u32 func_800256B4(s32 playerIndex);
void func_800291D8(s32 arg0);
s32 func_800291FC(void);
void func_8002BD58(s32 playerIndex, s32 strength, f32 duration);

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
#ifdef NON_MATCHING
void func_8001C114(s32 slotIndex, f32 x, f32 y, f32 z) {
    CameraOverrideSlot *slot;
    CameraTrackedObject *object;
    CameraTrackedObject **current;
    CameraBounds *bounds;
    f32 deltaX;
    f32 deltaZ;
    s32 index;

    if (slotIndex >= 0 && slotIndex < 4) {
        slot = &D_800CB368[slotIndex];
        object = slot->object;
        if (object != 0) {
            bounds = slot->bounds;
            if (bounds != 0) {
                deltaX = object->x - x;
                deltaZ = object->z - z;
                if ((bounds->trackedRadius * bounds->trackedRadius) <
                    ((deltaX * deltaX) + (deltaZ * deltaZ))) {
                    slot->object = 0;
                    goto clearExisting;
                }
                if ((bounds->flags & 0x8000) &&
                    ((y < bounds->trackedUpper) || (bounds->trackedLower < y))) {
                    slot->object = 0;
clearExisting:
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
                    deltaX = object->x - x;
                    deltaZ = object->z - z;
                    if (((deltaX * deltaX) + (deltaZ * deltaZ)) <
                        (bounds->radius * bounds->radius)) {
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
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/func_8001C2C4.s")
void func_8001C2D4(u8 *start, u8 *end) {
    u8 *current = start;

    if (start < end) {
        do {
            *current++ = 0;
        } while (current != end);
    }
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/func_8001C320.s")
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
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/func_8001D2A0.s")
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/func_8001D41C.s")
void controlFrozen(s32 arg0, ControlPlayer *player) {
    if (func_800291FC() == 1) {
        func_800291D8(10);
    }
    if (func_8002554C(player->playerIndex) & 0xF00F) {
        func_8001D690(arg0, player);
    }
}
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/func_8001D690.s")
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
#ifdef NON_MATCHING
void func_8001DCD0(s16 rotation, ControlVector3 *vector, s16 *pitch, s16 *yaw) {
    f32 cosine;
    f32 pitchX;
    f32 transformedX;
    f32 y;
    s32 angle;

    angle = -rotation;
    cosine = func_8002A8C0(angle);
    transformedX = func_8002A8BC(angle);
    y = vector->y;
    pitchX = (vector->z * cosine) + (vector->x * transformedX);
    transformedX = (vector->z * transformedX) - (vector->x * cosine);
    *pitch = Arctanf(-pitchX, y);
    *yaw = Arctanf(transformedX, y);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/charControl/func_8001DCD0.s")
#endif
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
        player->controlXjoy = func_800255DC(playerIndex);
        player->controlAbsXjoy = func_80025634(playerIndex);
        player->controlYjoy = func_8002565C(playerIndex);
        player->controlAbsYjoy = func_800256B4(playerIndex);
        player->controlKeys = func_800254FC(playerIndex);
        player->controlDkeys = func_8002554C(playerIndex);
        player->controlReleasedKeys = func_80025594(playerIndex);
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
        func_8002BD58(player->playerIndex, strength, duration);
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
