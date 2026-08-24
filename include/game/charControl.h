#ifndef _GAME_CHAR_CONTROL_H_
#define _GAME_CHAR_CONTROL_H_

#include "PR/ultratypes.h"

/* Partial camera-override layout; Mickey proves a 0x2C-byte record stride. */
typedef struct CameraOverride {
    /* 0x00 */ f32 blend;
    /* 0x04 */ u8 pad04[0x2C - 0x4];
} CameraOverride;

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

/* Partial player-control layout; fields are added only as Mickey proves them. */
typedef struct ControlPlayer {
    /* 0x000 */ s8 playerIndex;
    /* 0x001 */ u8 pad001[0x14 - 0x1];
    /* 0x014 */ f32 unk14[3];
    /* 0x020 */ u8 pad020[0x50 - 0x20];
    /* 0x050 */ f32 unk50;
    /* 0x054 */ f32 unk54;
    /* 0x058 */ u8 pad058[0x191 - 0x58];
    /* 0x191 */ s8 unk191;
    /* 0x192 */ u8 pad192[0x1A8 - 0x192];
    /* 0x1A8 */ u16 flags1A8;
    /* 0x1AA */ u8 pad1AA[0x2BC - 0x1AA];
    /* 0x2BC */ s32 unk2BC;
    /* 0x2C0 */ f32 unk2C0[(0x41C - 0x2C0) / sizeof(f32)];
    /* 0x41C */ s32 controlKeys;
    /* 0x420 */ s32 controlDkeys;
    /* 0x424 */ s32 controlReleasedKeys;
    /* 0x428 */ s32 controlXjoy;
    /* 0x42C */ s32 controlYjoy;
    /* 0x430 */ s32 controlAbsXjoy;
    /* 0x434 */ s32 controlAbsYjoy;
    /* 0x438 */ s32 joypadDisabled;
} ControlPlayer;

s16 dAngle(s16 arg0, s16 arg1, f32 arg2);
void controlFSUvels(s16 *rotation, ControlPlayer *player);
void controlDisableJoypad(ControlPlayer *player, s32 disabled);
void controlReadJoypad(ControlPlayer *player, s32 playerIndex);
void controlSetRumble(ControlPlayer *player, s32 strength, f32 duration);
void controlFrozen(s32 arg0, ControlPlayer *player);
void controlSetPlayerSetup(s16 arg0, s16 arg1, s16 arg2, s16 arg3);
s32 controlGetPlayerSetup(s16 *arg0, s16 *arg1, s16 *arg2, s16 *arg3);
void controlClearPlayerSetup(void);

#endif
