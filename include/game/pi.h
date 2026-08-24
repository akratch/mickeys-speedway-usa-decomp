#ifndef _GAME_PI_H_
#define _GAME_PI_H_

#include "PR/ultratypes.h"

/*
 * Cartridge asset DMA -- src/main/pi.c. Declared once here so every caller
 * shares one prototype instead of a per-file guess; src/main/models_5B300.c
 * and src/main/pi.c both include this header.
 */
s32 piRomLoadSection(u32 assetIndex, u32 address, s32 assetOffset, s32 size);

#endif
