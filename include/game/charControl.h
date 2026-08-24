#ifndef _GAME_CHAR_CONTROL_H_
#define _GAME_CHAR_CONTROL_H_

#include "PR/ultratypes.h"

/* Partial player-control layout; fields are added only as Mickey proves them. */
typedef struct ControlPlayer {
    /* 0x000 */ u8 pad000[0x438];
    /* 0x438 */ s32 joypadDisabled;
} ControlPlayer;

void controlDisableJoypad(ControlPlayer *player, s32 disabled);
void controlClearPlayerSetup(void);

#endif
