#ifndef _GAME_CHAR_CONTROL_H_
#define _GAME_CHAR_CONTROL_H_

#include "PR/ultratypes.h"

/* Partial camera-override layout; Mickey proves a 0x2C-byte record stride. */
typedef struct CameraOverride {
    /* 0x00 */ f32 blend;
    /* 0x04 */ u8 pad04[0x2C - 0x4];
} CameraOverride;

/* Camera-list records are indexed by a 0x54-byte stride. */
typedef struct ControlCameraState {
    /* 0x00 */ s16 unk0;
    /* 0x02 */ s16 unk2;
    /* 0x04 */ s16 unk4;
    /* 0x06 */ s16 unk6;
    /* 0x08 */ u8 pad08[0x0C - 0x08];
    /* 0x0C */ f32 x;
    /* 0x10 */ f32 y;
    /* 0x14 */ f32 z;
    /* 0x18 */ f32 unk18;
    /* 0x1C */ f32 unk1C;
    /* 0x20 */ f32 unk20;
    /* 0x24 */ f32 unk24;
    /* 0x28 */ f32 unk28;
    /* 0x2C */ u8 pad2C[0x3D - 0x2C];
    /* 0x3D */ u8 unk3D;
    /* 0x3E */ s16 unk3E;
    /* 0x40 */ f32 unk40;
    /* 0x44 */ u8 unk44;
    /* 0x45 */ u8 unk45;
    /* 0x46 */ u8 unk46;
    /* 0x47 */ u8 unk47;
    /* 0x48 */ u8 unk48;
    /* 0x49 */ u8 unk49;
    /* 0x4A */ u8 pad4A[0x54 - 0x4A];
} ControlCameraState;

typedef struct ControlTrackState {
    /* 0x00 */ u8 pad00[0x24];
    /* 0x24 */ s16 unk24;
} ControlTrackState;

typedef struct ControlLevelState {
    /* 0x000 */ u8 pad000[0x0E3];
    /* 0x0E3 */ u8 unk0E3;
    /* 0x0E4 */ u8 pad0E4[0x112 - 0x0E4];
    /* 0x112 */ s16 unk112[8];
} ControlLevelState;

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
    /* 0x08 */ f32 unk08;
    /* 0x0C */ f32 unk0C;
    /* 0x10 */ f32 unk10;
    /* 0x14 */ f32 unk14;
    /* 0x18 */ f32 unk18;
    /* 0x1C */ f32 unk1C;
    /* 0x20 */ f32 unk20;
    /* 0x24 */ f32 unk24;
    /* 0x28 */ f32 unk28;
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

/* Player point tables use four-float records with a 0x10-byte stride. */
typedef struct ControlGravityVector {
    /* 0x00 */ f32 x;
    /* 0x04 */ f32 y;
    /* 0x08 */ f32 z;
    /* 0x0C */ f32 w;
} ControlGravityVector;

typedef struct ControlParticleSlot {
    /* 0x00 */ u8 unk0;
    /* 0x01 */ u8 unk1;
    /* 0x02 */ u8 unk2;
    /* 0x03 */ u8 unk3;
    /* 0x04 */ u8 pad04[0x08 - 0x04];
    /* 0x08 */ void *handle;
} ControlParticleSlot;

typedef struct ControlActorAux {
    /* 0x00 */ u8 pad00[0x54];
    /* 0x54 */ f32 unk54;
} ControlActorAux;

typedef struct ControlParticleState {
    /* 0x00 */ u8 pad00[0x18];
    /* 0x18 */ f32 unk18;
} ControlParticleState;

typedef struct ControlParticleEffect {
    /* 0x00 */ u8 pad00[0x20];
    /* 0x20 */ f32 unk20;
    /* 0x24 */ u8 pad24[0x44 - 0x24];
    /* 0x44 */ s16 unk44;
    /* 0x46 */ u8 pad46[0x78 - 0x46];
    /* 0x78 */ ControlParticleState *state;
} ControlParticleEffect;

typedef struct ControlCollisionPlane {
    /* 0x00 */ f32 x;
    /* 0x04 */ f32 y;
    /* 0x08 */ f32 z;
    /* 0x0C */ f32 distance;
    /* 0x10 */ f32 unk10;
    /* 0x14 */ f32 unk14;
    /* 0x18 */ f32 unk18;
    /* 0x1C */ f32 unk1C;
    /* 0x20 */ s32 flags;
    /* 0x24 */ u8 kind;
    /* 0x25 */ u8 pad25[0x28 - 0x25];
} ControlCollisionPlane;

/* The collision callback state occupies D_800CB2C0 through D_800CB2FD. */
typedef struct ControlCollisionState {
    /* 0x00 */ void *hitObject;
    /* 0x04 */ f32 unk04;
    /* 0x08 */ f32 unk08;
    /* 0x0C */ f32 unk0C;
    /* 0x10 */ f32 unk10;
    /* 0x14 */ f32 unk14;
    /* 0x18 */ f32 unk18;
    /* 0x1C */ f32 unk1C;
    /* 0x20 */ f32 unk20;
    /* 0x24 */ f32 unk24;
    /* 0x28 */ f32 unk28;
    /* 0x2C */ f32 unk2C;
    /* 0x30 */ f32 unk30;
    /* 0x34 */ f32 unk34;
    /* 0x38 */ s32 flags;
    /* 0x3C */ u8 mode;
    /* 0x3D */ u8 state;
    /* 0x3E */ u8 pad3E[0x40 - 0x3E];
} ControlCollisionState;

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
    /* 0x40 */ u8 pad40[0x46 - 0x40];
    /* 0x46 */ s16 unk46;
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

typedef void (*ControlPlayerAction)(void *actor);

typedef struct ControlPlayerActions {
    /* 0x00 */ u8 pad00[0x10];
    /* 0x10 */ ControlPlayerAction positive;
    /* 0x14 */ ControlPlayerAction fallback;
    /* 0x18 */ ControlPlayerAction negative;
} ControlPlayerActions;

/* Partial player-control layout; fields are added only as Mickey proves them. */
typedef struct ControlPlayer {
    /* 0x000 */ s8 playerIndex;
    /* 0x001 */ s8 unk1;
    /* 0x002 */ u8 unk2;
    /* 0x003 */ u8 unk3;
    /* 0x004 */ f32 unk4;
    /* 0x008 */ f32 unk8;
    /* 0x00C */ f32 unkC;
    /* 0x010 */ f32 unk10;
    /* 0x014 */ f32 unk14[3];
    /* 0x020 */ u8 pad020[0x38 - 0x20];
    /* 0x038 */ f32 unk38;
    /* 0x03C */ f32 unk3C;
    /* 0x040 */ f32 unk40;
    /* 0x044 */ f32 unk44;
    /* 0x048 */ f32 unk48;
    /* 0x04C */ f32 unk4C;
    /* 0x050 */ f32 unk50;
    /* 0x054 */ f32 unk54;
    /* 0x058 */ u8 pad058[0x74 - 0x58];
    /* 0x074 */ f32 unk74;
    /* 0x078 */ f32 unk78;
    /* 0x07C */ f32 unk7C;
    /* 0x080 */ f32 unk80;
    /* 0x084 */ f32 unk84;
    /* 0x088 */ f32 unk88;
    /* 0x08C */ f32 unk8C;
    /* 0x090 */ f32 unk90;
    /* 0x094 */ u8 pad094[0xA4 - 0x94];
    /* 0x0A4 */ void *unkA4;
    /* 0x0A8 */ void *unkA8;
    /* 0x0AC */ void *unkAC;
    /* 0x0B0 */ u8 pad0B0[0xB4 - 0xB0];
    /* 0x0B4 */ void *unkB4;
    /* 0x0B8 */ u8 pad0B8[0xC8 - 0xB8];
    /* 0x0C8 */ void *unkC8;
    /* 0x0CC */ u8 pad0CC[0xD0 - 0xCC];
    /* 0x0D0 */ void *unkD0;
    /* 0x0D4 */ void *unkD4;
    /* 0x0D8 */ void *unkD8;
    /* 0x0DC */ s16 unkDC;
    /* 0x0DE */ u8 pad0DE[0xF0 - 0xDE];
    /* 0x0F0 */ s16 unkF0;
    /* 0x0F2 */ s16 unkF2;
    /* 0x0F4 */ s16 unkF4;
    /* 0x0F6 */ u8 pad0F6[0xFE - 0xF6];
    /* 0x0FE */ s16 unkFE;
    /* 0x100 */ u8 pad100[0x11C - 0x100];
    /* 0x11C */ f32 unk11C[4];
    /* 0x12C */ u8 pad12C[0x134 - 0x12C];
    /* 0x134 */ void *unk134;
    /* 0x138 */ u8 pad138[0x14C - 0x138];
    /* 0x14C */ f32 unk14C;
    /* 0x150 */ f32 unk150;
    /* 0x154 */ f32 unk154;
    /* 0x158 */ s16 unk158;
    /* 0x15A */ s16 unk15A;
    /* 0x15C */ s16 unk15C;
    /* 0x15E */ s16 unk15E;
    /* 0x160 */ s16 unk160;
    /* 0x162 */ s16 unk162;
    /* 0x164 */ s16 unk164;
    /* 0x166 */ s16 unk166;
    /* 0x168 */ s16 unk168;
    /* 0x16A */ s16 unk16A;
    /* 0x16C */ u8 unk16C;
    /* 0x16D */ u8 unk16D;
    /* 0x16E */ u8 pad16E;
    /* 0x16F */ u8 unk16F;
    /* 0x170 */ u8 pad170[0x172 - 0x170];
    /* 0x172 */ u8 unk172;
    /* 0x173 */ u8 unk173;
    /* 0x174 */ f32 unk174;
    /* 0x178 */ f32 unk178;
    /* 0x17C */ f32 unk17C;
    /* 0x180 */ u8 pad180[0x181 - 0x180];
    /* 0x181 */ u8 unk181;
    /* 0x182 */ u8 pad182[0x183 - 0x182];
    /* 0x183 */ u8 unk183;
    /* 0x184 */ u8 unk184;
    /* 0x185 */ u8 unk185;
    /* 0x186 */ u8 unk186;
    /* 0x187 */ u8 unk187;
    /* 0x188 */ f32 unk188;
    /* 0x18C */ u8 pad18C[0x18D - 0x18C];
    /* 0x18D */ s8 unk18D;
    /* 0x18E */ u8 unk18E;
    /* 0x18F */ u8 pad18F[0x190 - 0x18F];
    /* 0x190 */ u8 unk190;
    /* 0x191 */ s8 unk191;
    /* 0x192 */ u8 unk192;
    /* 0x193 */ u8 pad193[0x198 - 0x193];
    /* 0x198 */ u8 unk198;
    /* 0x199 */ u8 pad199[0x19A - 0x199];
    /* 0x19A */ u8 unk19A;
    /* 0x19B */ u8 unk19B;
    /* 0x19C */ s32 unk19C;
    /* 0x1A0 */ ControlPlayerActions *actions;
    /* 0x1A4 */ u8 unk1A4;
    /* 0x1A5 */ u8 unk1A5;
    /* 0x1A6 */ s16 unk1A6;
    /* 0x1A8 */ u16 flags1A8;
    /* 0x1AA */ u8 pad1AA[0x1B8 - 0x1AA];
    /* 0x1B8 */ s16 unk1B8;
    /* 0x1BA */ u8 pad1BA[0x2B8 - 0x1BA];
    /* 0x2B8 */ ControlGravityVector *unk2B8;
    /* 0x2BC */ s32 unk2BC;
    /* 0x2C0 */ f32 unk2C0[(0x2F0 - 0x2C0) / sizeof(f32)];
    /* 0x2F0 */ f32 unk2F0;
    /* 0x2F4 */ f32 unk2F4;
    /* 0x2F8 */ f32 unk2F8;
    /* 0x2FC */ u8 pad2FC[0x320 - 0x2FC];
    /* 0x320 */ u8 unk320;
    /* 0x321 */ u8 pad321[0x324 - 0x321];
    /* 0x324 */ s32 unk324;
    /* 0x328 */ u8 pad328[0x334 - 0x328];
    /* 0x334 */ void *unk334;
    /* 0x338 */ ControlParticleEffect *unk338;
    /* 0x33C */ s32 unk33C;
    /* 0x340 */ s32 unk340;
    /* 0x344 */ s32 unk344;
    /* 0x348 */ u8 unk348;
    /* 0x349 */ u8 unk349;
    /* 0x34A */ u8 unk34A;
    /* 0x34B */ u8 unk34B;
    /* 0x34C */ ControlParticleSlot particles[4];
    /* 0x37C */ u8 pad37C[0x387 - 0x37C];
    /* 0x387 */ u8 unk387;
    /* 0x388 */ u8 unk388;
    /* 0x389 */ u8 pad389[0x3BA - 0x389];
    /* 0x3BA */ s16 unk3BA;
    /* 0x3BC */ u8 pad3BC[0x3EC - 0x3BC];
    /* 0x3EC */ f32 unk3EC;
    /* 0x3F0 */ f32 unk3F0;
    /* 0x3F4 */ u8 pad3F4[0x3FA - 0x3F4];
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
    /* 0x454 */ u8 pad454[0x456 - 0x454];
    /* 0x456 */ s16 unk456;
    /* 0x458 */ u8 pad458[0x45C - 0x458];
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
    /* 0x3A */ s8 unk3A;
    /* 0x3B */ u8 pad3B[0x44 - 0x3B];
    /* 0x44 */ s16 kind;
    /* 0x46 */ u8 pad46[0x48 - 0x46];
    /* 0x48 */ ControlActorAux *unk48;
    /* 0x4C */ u8 pad4C[0x64 - 0x4C];
    /* 0x64 */ ControlPlayer *player;
    /* 0x68 */ void ***unk68;
    /* 0x6C */ u8 pad6C[0x70 - 0x6C];
    /* 0x70 */ s32 *unk70;
    /* 0x74 */ u8 pad74[0x80 - 0x74];
    /* 0x80 */ s32 unk80;
} ControlActor;

s16 dAngle(s16 arg0, s16 arg1, f32 arg2);
void controlFSUvels(s16 *rotation, ControlPlayer *player);
void controlDisableJoypad(ControlPlayer *player, s32 disabled);
void controlReadJoypad(ControlPlayer *player, s32 playerIndex);
void controlSetRumble(ControlPlayer *player, s32 strength, f32 duration);
void controlFrozen(ControlActor *actor, ControlPlayer *player);
void func_8001D2A0(ControlActor *actor, s32 arg1);
void func_8001D41C(ControlActor *actor, ControlPlayer *player, s32 updateRate);
void func_8001D690(ControlActor *actor, ControlPlayer *player);
void controlPlayerReInit(ControlActor *actor, f32 x, f32 y, f32 z,
                         s16 arg4, s16 arg5, s16 arg6);
void controlSetPlayerSetup(s16 arg0, s16 arg1, s16 arg2, s16 arg3);
s32 controlGetPlayerSetup(s16 *arg0, s16 *arg1, s16 *arg2, s16 *arg3);
void controlClearPlayerSetup(void);

#endif
