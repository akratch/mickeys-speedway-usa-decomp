#ifndef _GAME_CHAR_CONTROL_H_
#define _GAME_CHAR_CONTROL_H_

#include "PR/ultratypes.h"

/* Partial camera-override layout; Mickey proves a 0x2C-byte record stride. */
typedef struct CameraOverride {
    /* 0x00 */ f32 blend;
    /* 0x04 */ u8 pad04[0x2C - 0x4];
} CameraOverride;

typedef struct CameraBounds {
    /* 0x00 */ f32 radius;
    /* 0x04 */ f32 lower;
    /* 0x08 */ f32 upper;
    /* 0x0C */ f32 trackedRadius;
    /* 0x10 */ f32 trackedLower;
    /* 0x14 */ f32 trackedUpper;
    /* 0x18 */ u32 flags;
} CameraBounds;

typedef struct CameraTrackedObject {
    /* 0x00 */ u8 pad00[0x0C];
    /* 0x0C */ f32 x;
    /* 0x10 */ u8 pad10[0x14 - 0x10];
    /* 0x14 */ f32 z;
    /* 0x18 */ u8 pad18[0x64 - 0x18];
    /* 0x64 */ CameraBounds *bounds;
} CameraTrackedObject;

typedef struct CameraOverrideSlot {
    /* 0x00 */ CameraTrackedObject *object;
    /* 0x04 */ CameraBounds *bounds;
    /* 0x08 */ u8 pad08[0x2C - 0x08];
} CameraOverrideSlot;

/* Partial transform input; Mickey proves the translation vector at 0x0C. */
typedef struct ControlTransform {
    /* 0x00 */ u8 pad00[0x0C];
    /* 0x0C */ f32 x;
    /* 0x10 */ f32 y;
    /* 0x14 */ f32 z;
} ControlTransform;

typedef struct ControlVector3 {
    /* 0x00 */ f32 x;
    /* 0x04 */ f32 y;
    /* 0x08 */ f32 z;
} ControlVector3;

typedef struct ControlSpawnPacket {
    /* 0x00 */ s16 kind;
    /* 0x02 */ u8 mode;
    /* 0x03 */ u8 flags;
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ s16 unkA;
} ControlSpawnPacket;

typedef struct ControlSpawned {
    /* 0x00 */ u8 pad00[0x3C];
    /* 0x3C */ s32 unk3C;
} ControlSpawned;

typedef struct ControlCeilingContext {
    /* 0x00 */ u8 pad00[0x68];
    /* 0x68 */ f32 height;
    /* 0x6C */ u8 pad6C[0xB0 - 0x6C];
    /* 0xB0 */ void *handle;
} ControlCeilingContext;

typedef struct ControlPlayerInitState {
    /* 0x00 */ u8 pad00[0x0A];
    /* 0x0A */ s16 arg6;
    /* 0x0C */ s16 arg5;
    /* 0x0E */ s16 arg4;
    /* 0x10 */ s8 playerIndex;
    /* 0x11 */ s8 unk11;
} ControlPlayerInitState;

/* Partial player-control layout; fields are added only as Mickey proves them. */
typedef struct ControlPlayer {
    /* 0x000 */ s8 playerIndex;
    /* 0x001 */ s8 unk1;
    /* 0x002 */ u8 pad002[0x14 - 0x2];
    /* 0x014 */ f32 unk14[3];
    /* 0x020 */ u8 pad020[0x50 - 0x20];
    /* 0x050 */ f32 unk50;
    /* 0x054 */ f32 unk54;
    /* 0x058 */ u8 pad058[0xD0 - 0x58];
    /* 0x0D0 */ void *unkD0;
    /* 0x0D4 */ void *unkD4;
    /* 0x0D8 */ void *unkD8;
    /* 0x0DC */ u8 pad0DC[0x11C - 0xDC];
    /* 0x11C */ f32 unk11C[4];
    /* 0x12C */ u8 pad12C[0x14C - 0x12C];
    /* 0x14C */ f32 unk14C;
    /* 0x150 */ u8 pad150[0x154 - 0x150];
    /* 0x154 */ f32 unk154;
    /* 0x158 */ s16 unk158;
    /* 0x15A */ u8 pad15A[0x160 - 0x15A];
    /* 0x160 */ s16 unk160;
    /* 0x162 */ s16 unk162;
    /* 0x164 */ s16 unk164;
    /* 0x166 */ u8 pad166[0x190 - 0x166];
    /* 0x190 */ u8 unk190;
    /* 0x191 */ s8 unk191;
    /* 0x192 */ u8 unk192;
    /* 0x193 */ u8 pad193[0x1A8 - 0x193];
    /* 0x1A8 */ u16 flags1A8;
    /* 0x1AA */ u8 pad1AA[0x2B8 - 0x1AA];
    /* 0x2B8 */ ControlVector3 *unk2B8;
    /* 0x2BC */ s32 unk2BC;
    /* 0x2C0 */ f32 unk2C0[(0x33C - 0x2C0) / sizeof(f32)];
    /* 0x33C */ s32 unk33C;
    /* 0x340 */ s32 unk340;
    /* 0x344 */ u8 pad344[0x3BA - 0x344];
    /* 0x3BA */ s16 unk3BA;
    /* 0x3BC */ u8 pad3BC[0x3FA - 0x3BC];
    /* 0x3FA */ s16 unk3FA;
    /* 0x3FC */ u8 pad3FC[0x41C - 0x3FC];
    /* 0x41C */ s32 controlKeys;
    /* 0x420 */ s32 controlDkeys;
    /* 0x424 */ s32 controlReleasedKeys;
    /* 0x428 */ s32 controlXjoy;
    /* 0x42C */ s32 controlYjoy;
    /* 0x430 */ s32 controlAbsXjoy;
    /* 0x434 */ s32 controlAbsYjoy;
    /* 0x438 */ s32 joypadDisabled;
    /* 0x43C */ s16 unk43C;
    /* 0x43E */ s16 unk43E;
    /* 0x440 */ s16 unk440;
    /* 0x442 */ u8 pad442[0x444 - 0x442];
    /* 0x444 */ f32 unk444;
    /* 0x448 */ f32 unk448;
    /* 0x44C */ f32 unk44C;
    /* 0x450 */ f32 unk450;
    /* 0x454 */ u8 pad454[0x45C - 0x454];
    /* 0x45C */ u8 unk45C;
    /* 0x45D */ u8 unk45D;
} ControlPlayer;

typedef struct ControlActor {
    /* 0x00 */ s16 rotationX;
    /* 0x02 */ s16 rotationY;
    /* 0x04 */ s16 rotationZ;
    /* 0x06 */ s16 flags;
    /* 0x08 */ f32 unk8;
    /* 0x0C */ f32 x;
    /* 0x10 */ f32 y;
    /* 0x14 */ f32 z;
    /* 0x18 */ u8 pad18[0x1C - 0x18];
    /* 0x1C */ f32 velocityX;
    /* 0x20 */ f32 velocityY;
    /* 0x24 */ f32 velocityZ;
    /* 0x28 */ u8 pad28[0x2E - 0x28];
    /* 0x2E */ s16 positionTag;
    /* 0x30 */ u8 pad30[0x39 - 0x30];
    /* 0x39 */ u8 alpha;
    /* 0x3A */ u8 pad3A[0x44 - 0x3A];
    /* 0x44 */ s16 kind;
    /* 0x46 */ u8 pad46[0x64 - 0x46];
    /* 0x64 */ ControlPlayer *player;
    /* 0x68 */ u8 pad68[0x80 - 0x68];
    /* 0x80 */ s32 unk80;
} ControlActor;

s16 dAngle(s16 arg0, s16 arg1, f32 arg2);
void controlFSUvels(s16 *rotation, ControlPlayer *player);
void controlDisableJoypad(ControlPlayer *player, s32 disabled);
void controlReadJoypad(ControlPlayer *player, s32 playerIndex);
void controlSetRumble(ControlPlayer *player, s32 strength, f32 duration);
void controlFrozen(ControlActor *actor, ControlPlayer *player);
void func_8001D2A0(ControlActor *actor, s32 arg1);
void func_8001D690(ControlActor *actor, ControlPlayer *player);
void controlPlayerReInit(ControlActor *actor, f32 x, f32 y, f32 z,
                         s16 arg4, s16 arg5, s16 arg6);
void controlSetPlayerSetup(s16 arg0, s16 arg1, s16 arg2, s16 arg3);
s32 controlGetPlayerSetup(s16 *arg0, s16 *arg1, s16 *arg2, s16 *arg3);
void controlClearPlayerSetup(void);

#endif
